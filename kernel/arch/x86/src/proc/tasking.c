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
extern void *get_eip();      //!< Get the EIP value of the instruction after the call to this function

void arch_multitasking_init(void) {
	set_interrupt(INTR_SCHED, sched_run);
}

uintptr_t arch_proc_create_stack(kthread_t *thread, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel) {
/*#ifdef STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size + 0x2000);
#else // STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size);
#endif // !STACK_PROTECTOR*/

	// TODO: Use better method, so as not to waste 4K of memory for every process.
    uintptr_t stack_begin = (uintptr_t)kmamalloc(stack_size, 4096);

	kdebug(DEBUGSRC_PROC, "proc_create_stack [size: %d] [end: %08X, beg: %08X]",
		stack_size, stack_begin, stack_begin + stack_size
	);


    // Map stack memory
	for(size_t i = 0; i < stack_size; i+= 0x1000) {
		if(is_kernel) pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
		else          pgdir_map_page((uint32_t *)thread->process->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
	}

	thread->arch.stack_end = virt_stack_begin;
	thread->arch.stack_beg = virt_stack_begin + stack_size;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	block_page(thread->stack_end - 0x1000);
	block_page(thread->stack_beg);
#endif // STACK_PROTECTOR

	return stack_begin;
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

void proc_jump_to_ring(void) {
	/* TODO: Select proper CPU */
	kthread_t *curr_thread = sched_get_curr_thread(0);
    kproc_t   *curr_proc   = curr_thread->process;
	
	if(curr_proc && curr_thread) {
		if(curr_thread->entrypoint) {
			enter_ring(curr_proc->arch.ring, (void *)curr_thread->entrypoint);
		}
	}
}

__noreturn
static void _thread_entrypoint(void (*entrypoint)(void *), void *data) {
	entrypoint(data);
	exit(1);
}

int arch_setup_thread(kthread_t *thread, void *entrypoint, uint32_t stack_size, void *data) {
	/* @todo */
	(void)data;

	int kernel = (thread->process->type & TYPE_KERNEL);
	
	thread->arch.eip   = (uint32_t)_thread_entrypoint;

	uint32_t /*stack_begin, */virt_stack_begin;
	if(!kernel) virt_stack_begin = 0xFF000000;
	else        virt_stack_begin = 0x7F000000;
	/* @todo Find better scheme to separate thread stacks. */
	int tpos = llist_get_position(&thread->process->threads, &thread->list_item);
	virt_stack_begin -= tpos * 0x10000;
	uintptr_t stack = proc_create_stack(thread, stack_size, virt_stack_begin, kernel);
	proc_create_kernel_stack(thread);

	/* Setup call stack for _thread_entrypoint */
	*(uint32_t *)(stack + stack_size - 4)  = (uint32_t)data;
	*(uint32_t *)(stack + stack_size - 8)  = (uint32_t)entrypoint;
	*(uint32_t *)(stack + stack_size - 12) = (uint32_t)NULL;
	thread->arch.esp = (thread->arch.ebp = virt_stack_begin + stack_size) - 12;

	if(kernel == 0) {
		thread->arch.eip = (uint32_t)proc_jump_to_ring;
	}

	kdebug(DEBUGSRC_PROC, "arch_setup_thread EIP: %08X CR3: %08X ESP: %08X", thread->arch.eip, thread->process->arch.cr3, thread->arch.esp);

    return 0;
}

int arch_setup_task(kthread_t *thread, void *entrypoint, uint32_t stack_size, arch_task_params_t *arch_params) {
	thread->process->arch.ring  = arch_params->ring;
	thread->process->arch.cr3   = (uint32_t)arch_params->pgdir;

	arch_setup_thread(thread, entrypoint, stack_size, NULL);

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
		sched_processes();
		return;
	}

	if(thread->flags & KTHREAD_FLAG_RANONCE) {
		thread->arch.esp = esp;
		thread->arch.ebp = ebp;
		thread->arch.eip = eip;
	}
	else thread->flags |= KTHREAD_FLAG_RANONCE;

    /*kdebug(DEBUGSRC_PROC, "-TID: %d | PC: %08X | SP: %08X | BLK: %08X | TYPE: %08X | FLAG: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->blocked, proc->type, thread->flags, thread->name);*/
	
	// Switch to next process here...
	thread = sched_next_process(0);
	proc   = thread->process;
    
    /*kdebug(DEBUGSRC_PROC, "+TID: %d | PC: %08X | SP: %08X | BLK: %08X | TYPE: %08X | FLAG: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->blocked, proc->type, thread->flags, thread->name);*/

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
