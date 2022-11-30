#include <arch/intr/syscall.h>
#include <arch/proc/stack_trace.h>

#include <intr/types/intr.h>
#include <proc/mtask.h>
#include <proc/syscalls.h>

void arch_syscall_interrupt(intr_handler_hand_t *hdlr) {
    kthread_t *curr_thread = mtask_get_curr_thread();

    curr_thread->arch.esp = (uint32_t)&hdlr->arch.pregs;

    curr_thread->arch.syscall_regs.pusha = hdlr->arch.pregs;
    curr_thread->arch.syscall_regs.iret  = hdlr->arch.iregs;

    uint32_t  scn  = hdlr->arch.pregs->eax;
    uint32_t *args = (uint32_t *)hdlr->arch.pregs->ebx;

    if(syscall_service(scn, args)) {
        stack_trace(15, (uint32_t *)hdlr->arch.pregs->ebp, hdlr->arch.iregs->eip, curr_thread->process->symbols);
        exit(1);
    }
}

