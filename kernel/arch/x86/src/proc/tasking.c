#include <arch/proc/tasking.h>
#include <arch/proc/user.h>
#include <arch/intr/int.h>
#include <arch/mm/paging.h>
#include <arch/mm/gdt.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <intr/intr.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <types.h>

extern lock_t creat_task; // From proc/mtask.c

extern void sched_run(void);
extern void *get_eip(void);  //!< Get the EIP value of the instruction after the call to this function

void arch_multitasking_init(void) {
	set_interrupt(INTR_SCHED, sched_run);
}

int arch_proc_create_stack(kthread_t *thread) {
/*#ifdef STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size + 0x2000);
#else // STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size);
#endif // !STACK_PROTECTOR*/

	if(!SAFETY_CHECK(thread->process)) {
		kpanic("arch_proc_create_stack: Thread has no associated process!");
	}

	int kernel = thread->process->type & TYPE_KERNEL;
	
	uint32_t virt_stack_begin = 0x7F000000;
	
	/* @todo Find better scheme to separate thread stacks. */
	int tpos = llist_get_position(&thread->process->threads, &thread->list_item);
	virt_stack_begin -= tpos * 0x10000;

	// TODO: Use better method, so as not to waste 4K of memory for every process.
    uintptr_t stack_begin = (uintptr_t)kmamalloc(thread->stack_size, 4096);

	kdebug(DEBUGSRC_PROC, "arch_proc_create_stack [size: %d] [end: %08X, beg: %08X]",
		thread->stack_size, stack_begin, stack_begin + thread->stack_size
	);


    // Map stack memory
	for(size_t i = 0; i < thread->stack_size; i+= 0x1000) {
		if(kernel) pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
		else       pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
	}

	thread->arch.stack_end = virt_stack_begin;
	thread->arch.stack_beg = virt_stack_begin + thread->stack_size;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	block_page(thread->stack_end - 0x1000);
	block_page(thread->stack_beg);
#endif // STACK_PROTECTOR
	
	thread->arch.ebp = thread->arch.stack_beg;

	return 0;
}

int arch_proc_create_kernel_stack(kthread_t *thread) {
	thread->arch.kernel_stack = (uint32_t)kmamalloc(PROC_KERN_STACK_SIZE, 4096);

	kdebug(DEBUGSRC_PROC, "arch_proc_create_kernel_stack [size: %d] [end: %08X, beg: %08X]",
		PROC_KERN_STACK_SIZE, thread->arch.kernel_stack, thread->arch.kernel_stack + PROC_KERN_STACK_SIZE
	);

	for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
		pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)(thread->arch.kernel_stack + i), (void *)(thread->arch.kernel_stack + i), 0x03);
	}

	thread->arch.kernel_stack += PROC_KERN_STACK_SIZE;

    return 0;
}

static void _thread_entrypoint(void) {
	kthread_t *curr_thread = sched_get_curr_thread(0);
	if(curr_thread == NULL) {
		kpanic("_proc_entrypoint: Thread is NULL!");
	}

    kproc_t *curr_proc = curr_thread->process;
	if(curr_proc == NULL) {
		kpanic("_proc_entrypoint: Process is NULL!");
	}
	if(curr_thread->entrypoint == 0) {
		kpanic("_proc_entrypoint: Entrypoint is NULL!");
	}

	if(curr_proc->type & TYPE_KERNEL) {	
		STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->thread_data);
	} else {
		/* Empty args and env for CRT0 w/ user applications */
		STACK_PUSH(curr_thread->arch.stack_entry, NULL);                            /* ENVP */
		STACK_PUSH(curr_thread->arch.stack_entry, NULL);                            /* ARGV */
		STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->arch.stack_beg - 4); /* ENVP ptr */
		STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->arch.stack_beg - 8); /* ARGV ptr */
		STACK_PUSH(curr_thread->arch.stack_entry, 0);                               /* ARGC */
	}

	enter_ring_newstack(curr_proc->arch.ring, (void *)curr_thread->entrypoint, (void *)curr_thread->arch.stack_entry);
	
	exit(1);
}

int arch_setup_thread(kthread_t *thread) {
	proc_create_stack(thread);
	proc_create_kernel_stack(thread);
	
	thread->arch.eip         = (uint32_t)_thread_entrypoint;
	thread->arch.esp         = thread->arch.kernel_stack;
	thread->arch.stack_entry = thread->arch.stack_beg;

	kdebug(DEBUGSRC_PROC, "arch_setup_thread EIP: %08X CR3: %08X ESP: %08X", thread->arch.eip, thread->process->arch.cr3, thread->arch.esp);

    return 0;
}

int arch_setup_process(kproc_t __unused *proc) {
    return 0;
}


__hot void do_task_switch(void) {
	/* TODO: Select proper CPU */
	kthread_t *thread = sched_get_curr_thread(0);
	if(!thread) {
		return;
	}
	kproc_t   *proc   = thread->process;

	uint32_t esp, ebp, eip, cr3;
	asm volatile ("mov %%esp, %0" : "=r" (esp));
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = (uint32_t)get_eip();

	if(eip == 0xFFFFFFFF) {
		return;
	}

	if(thread->flags & KTHREAD_FLAG_RANONCE) {
		thread->arch.esp = esp;
		thread->arch.ebp = ebp;
		thread->arch.eip = eip;
	}

    //kdebug(DEBUGSRC_PROC, "-TID: %d | PC: %08X | SP: %08X | CR3: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->process->arch.cr3, thread->name);
	
	// Switch to next process here...
	thread = sched_next_process(0);
	proc   = thread->process;
    
    //kdebug(DEBUGSRC_PROC, "+TID: %d | PC: %08X | SP: %08X | CR3: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->process->arch.cr3, thread->name);

	thread->flags |= KTHREAD_FLAG_RANONCE;
	if (!thread->arch.kernel_stack) {	
		kpanic("do_task_switch: No kernel stack set for thread!");
	}
	tss_set_kern_stack(thread->arch.kernel_stack);

	esp = thread->arch.esp;
	ebp = thread->arch.ebp;
	eip = thread->arch.eip;
	cr3 = proc->arch.cr3;

	asm volatile("mov %0, %%ebx\n"
				 "mov %1, %%esp\n"
				 "mov %2, %%ebp\n"
				 "mov %3, %%cr3\n"
				 "mov $0xFFFFFFFF, %%eax\n"
				 "sti\n"
				 "jmp *%%ebx"
				 : : "r" (eip), "r" (esp), "r" (ebp), "r" (cr3)
				 : "%ebx", /*"%esp", */"%eax");
}
