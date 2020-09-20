#include <arch/proc/user.h>
#include <arch/intr/int.h>
#include <arch/mm/paging.h>
#include <arch/mm/gdt.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <intr/intr.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <types.h>

extern lock_t creat_task; // From proc/mtask.c

extern void sched_run(void);
extern void *get_eip();      //!< Get the EIP value of the instruction after the call to this function

static int c_proc = 0; //!< Index of current process

void arch_multitasking_init(void) {
	set_interrupt(INT_SCHED, sched_run);
}

int arch_proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel) {
/*#ifdef STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size + 0x2000);
#else // STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size);
#endif // !STACK_PROTECTOR*/

	// TODO: Use better method, so as not to waste 4K of memory for every process.
    uintptr_t stack_begin = (uintptr_t)kmalloc(stack_size + 4096);
	stack_begin = (stack_begin + 4096) & 0xFFFFF000;

	kerror(ERR_BOOTINFO, "proc_create_stack [size: %d] [end: %08X, beg: %08X]",
		stack_size, stack_begin, stack_begin + stack_size
	);


    // Map stack memory
	for(size_t i = 0; i < stack_size; i+= 0x1000) {
		if(is_kernel) pgdir_map_page((uint32_t *)proc->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
		else          pgdir_map_page((uint32_t *)proc->arch.cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
	}

	proc->arch.stack_end = virt_stack_begin;
	proc->arch.stack_beg = virt_stack_begin + stack_size;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	block_page(proc->stack_end - 0x1000);
	block_page(proc->stack_beg);
#endif // STACK_PROTECTOR

	return 0;
}

int arch_proc_create_kernel_stack(struct kproc *proc) {
	proc->arch.kernel_stack = (uint32_t)kmalloc(PROC_KERN_STACK_SIZE + 4096);
	proc->arch.kernel_stack = (proc->arch.kernel_stack + 4096) & 0xFFFFF000;

	kerror(ERR_BOOTINFO, "proc_create_kernel_stack [size: %d] [end: %08X, beg: %08X]",
		PROC_KERN_STACK_SIZE, proc->arch.kernel_stack, proc->arch.kernel_stack + PROC_KERN_STACK_SIZE
	);

	for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
		pgdir_map_page((uint32_t *)proc->arch.cr3, (void *)(proc->arch.kernel_stack + i), (void *)(proc->arch.kernel_stack + i), 0x03);
	}

	proc->arch.kernel_stack += PROC_KERN_STACK_SIZE;

    return 0;
}

void proc_jump_to_ring(void) {
	int p = proc_by_pid(current_pid);
	if(p > 0) {
		if(procs[c_proc].entrypoint) {
			enter_ring(procs[c_proc].arch.ring, (void *)procs[c_proc].entrypoint);
		}
	}
}

int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, int kernel, arch_task_params_t *arch_params) {
	proc->arch.ring  = arch_params->ring;
	proc->arch.eip   = (uint32_t)entrypoint;
	proc->entrypoint = (uint32_t)entrypoint;
	proc->arch.cr3   = (uint32_t)arch_params->pgdir;

	uint32_t /*stack_begin, */virt_stack_begin;
	if(!kernel) virt_stack_begin = 0xFF000000;
	else        virt_stack_begin = 0x7F000000;
	proc_create_stack(proc, stack_size, virt_stack_begin, kernel);


	proc->arch.esp = proc->arch.ebp = virt_stack_begin + stack_size;

	if(kernel == 0) {
		/*STACK_PUSH(stack_begin, 0xFFFFAAAA);//process);
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		STACK_PUSH(stack_begin, ring);
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		STACK_PUSH(stack_begin, 0xFEAFBEEF); // Return Address
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		procs[p].eip = (uint32_t)enter_ring;
		procs[p].esp -= 12;*/
		proc->arch.eip = (uint32_t)proc_jump_to_ring;
	}

    return 0;
}






__hot void do_task_switch(void) {
	if(!tasking)   return;
	if(creat_task) return; // We don't want to interrupt process creation

#if  defined(ARCH_X86)
	uint32_t esp, ebp, eip, cr3;
	asm volatile ("mov %%esp, %0" : "=r" (esp));
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = (uint32_t)get_eip();

	if(eip == 0xFFFFFFFF) {
		sched_processes();
		return;
	}
#endif


	if(procs[c_proc].type & TYPE_RANONCE) {
#if  defined(ARCH_X86)
		procs[c_proc].arch.esp = esp;
		procs[c_proc].arch.ebp = ebp;
		procs[c_proc].arch.eip = eip;
#endif
	}
	else procs[c_proc].type |= TYPE_RANONCE;

	// Switch to next process here...
	c_proc = sched_next_process();


#if  defined(ARCH_X86)
	tss_set_kern_stack(procs[c_proc].arch.kernel_stack);

	esp = procs[c_proc].arch.esp;
	ebp = procs[c_proc].arch.ebp;
	eip = procs[c_proc].arch.eip;
	cr3 = procs[c_proc].arch.cr3;

	asm volatile("mov %0, %%ebx\n"
				 "mov %1, %%esp\n"
				 "mov %2, %%ebp\n"
				 "mov %3, %%cr3\n"
				 "mov $0xFFFFFFFF, %%eax\n"
				 "sti\n"
				 "jmp *%%ebx"
				 : : "r" (eip), "r" (esp), "r" (ebp), "r" (cr3)
				 : "%ebx", /*"%esp", */"%eax");
#endif
}
