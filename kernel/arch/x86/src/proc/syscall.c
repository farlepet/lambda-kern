#include <arch/intr/int.h>
#include <arch/proc/stack_trace.h>

#include <proc/syscalls.h>
#include <proc/mtask.h>
#include <types.h>

void handle_syscall(struct pusha_regs regs, struct iret_regs iregs) {
	(void)iregs;
	
	struct kproc *proc = &procs[proc_by_pid(current_pid)];
	proc->arch.esp = (uint32_t)&regs;
	proc->book.syscall_count++;

	proc->arch.syscall_regs.pusha = &regs;
	proc->arch.syscall_regs.iret  = &iregs;

	uint32_t  scn  = regs.eax;
	uint32_t *args = (uint32_t *)regs.ebx;

	if(service_syscall(scn, args)) {
		stack_trace(15, (uint32_t *)regs.ebp, iregs.eip, proc->symbols);
		exit(1);
	}


}