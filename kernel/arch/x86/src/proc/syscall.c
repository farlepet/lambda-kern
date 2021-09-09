#include <arch/intr/int.h>
#include <arch/proc/stack_trace.h>

#include <proc/syscalls.h>
#include <proc/mtask.h>
#include <types.h>

void handle_syscall(struct pusha_regs regs, struct iret_regs iregs) {
	(void)iregs;
    
	kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = curr_thread->process;
	
	curr_thread->arch.esp = (uint32_t)&regs;
	curr_proc->book.syscall_count++;

	curr_thread->arch.syscall_regs.pusha = &regs;
	curr_thread->arch.syscall_regs.iret  = &iregs;

	uint32_t  scn  = regs.eax;
	uint32_t *args = (uint32_t *)regs.ebx;

	if(service_syscall(scn, args)) {
		stack_trace(15, (uint32_t *)regs.ebp, iregs.eip, curr_proc->symbols);
		exit(1);
	}


}