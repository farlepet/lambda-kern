#include <arch/proc/tasking.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>

extern lock_t creat_task; // From proc/mtask.c

extern uint32_t irq_stack_end[];


int arch_proc_create_stack(kthread_t *thread) {
    /* Might need to increase alignment when incorporating MMU */
    uint32_t stack = (uint32_t)kamalloc(thread->stack_size, 16);
    if(!stack) {
        return -1;
    }

    /* TODO: MMU */
    thread->arch.stack_beg = stack;
    thread->arch.stack_end = stack + thread->stack_size;

    return 0;
}

int arch_proc_create_kernel_stack(kthread_t __unused *thread) {
    /* NOTE: At least at present, kernel stack is shared by all processes. */
    return 0;
}

int arch_setup_thread(kthread_t *thread) {
    thread->arch.regs.pc = (uint32_t)thread->entrypoint;

    proc_create_stack(thread);

    /* TODO: Support multiple modes apart from supervisor */
    thread->arch.regs.cpsr = 0x60000113;
    thread->arch.regs.sp = thread->arch.stack_end;

    return 0;
}

int arch_setup_process(kproc_t __unused *thread) {
    return 0;
}

void arch_multitasking_init(void) {
    /* TODO */
}

__hot void do_task_switch(void) {
	kthread_t *thread = sched_get_curr_thread(0);

    if(thread->flags & KTHREAD_FLAG_RANONCE) {
        /* Save registers */
        /* push {lr}
         * push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
         * mrs  r0, spsr
         * push {r0} */
        /* TODO: Cleanup */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        thread->arch.regs.pc   = irq_stack_end[-1];
        thread->arch.regs.r0   = irq_stack_end[-2];
        thread->arch.regs.r1   = irq_stack_end[-3];
        thread->arch.regs.r2   = irq_stack_end[-4];
        thread->arch.regs.r3   = irq_stack_end[-5];
        thread->arch.regs.r4   = irq_stack_end[-6];
        thread->arch.regs.r5   = irq_stack_end[-7];
        thread->arch.regs.r6   = irq_stack_end[-8];
        thread->arch.regs.r7   = irq_stack_end[-9];
        thread->arch.regs.r8   = irq_stack_end[-10];
        thread->arch.regs.r9   = irq_stack_end[-11];
        thread->arch.regs.r10  = irq_stack_end[-12];
        thread->arch.regs.r11  = irq_stack_end[-13];
        thread->arch.regs.r12  = irq_stack_end[-14];
        thread->arch.regs.cpsr = irq_stack_end[-15];
#pragma GCC diagnostic pop

        /* TODO: Support multiple modes apart from supervisor */
        /* Save SP and LR: */
        asm volatile("mrs r0, cpsr \n"
                     "orr r0, r0, #0x13 \n"
                     "msr cpsr, r0 \n"

                     "mov %0, sp \n"
                     "mov %1, lr \n"

                     "bic r0, r0, #0x0d \n"
                     "msr cpsr, r0 \n"
                     : "=r" (thread->arch.regs.sp), "=r" (thread->arch.regs.lr));
    } else {
        thread->flags |= KTHREAD_FLAG_RANONCE;
    }
    
    //kdebug(DEBUGSRC_PROC, "-TID: %d | PC: %08X | LR: %08X | SP: %08X | CPSR: %08X | NAME: %s", thread->tid, thread->arch.regs.pc, thread->arch.regs.lr, thread->arch.regs.sp, thread->arch.regs.cpsr, thread->name);

    /* Get next threadess to run. */
	thread = sched_next_process(0);

    /* Load registers: */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    irq_stack_end[-1]  = thread->arch.regs.pc;
    irq_stack_end[-2]  = thread->arch.regs.r0;
    irq_stack_end[-3]  = thread->arch.regs.r1;
    irq_stack_end[-4]  = thread->arch.regs.r2;
    irq_stack_end[-5]  = thread->arch.regs.r3;
    irq_stack_end[-6]  = thread->arch.regs.r4;
    irq_stack_end[-7]  = thread->arch.regs.r5;
    irq_stack_end[-8]  = thread->arch.regs.r6;
    irq_stack_end[-9]  = thread->arch.regs.r7;
    irq_stack_end[-10] = thread->arch.regs.r8;
    irq_stack_end[-11] = thread->arch.regs.r9;
    irq_stack_end[-12] = thread->arch.regs.r10;
    irq_stack_end[-13] = thread->arch.regs.r11;
    irq_stack_end[-14] = thread->arch.regs.r12;
    irq_stack_end[-15] = thread->arch.regs.cpsr;
#pragma GCC diagnostic pop
	
    //kdebug(DEBUGSRC_PROC, "+TID: %d | PC: %08X | LR: %08X | SP: %08X | CPSR: %08X | NAME: %s", thread->tid, thread->arch.regs.pc, thread->arch.regs.lr, thread->arch.regs.sp, thread->arch.regs.cpsr, thread->name);

    /* Restore SP and LR: */
    asm volatile("mrs r0, cpsr \n"
                 "orr r0, r0, #0x13 \n"
                 "msr cpsr, r0 \n"

                 "mov sp, %0 \n"
                 "mov lr, %1 \n"

                 "bic r0, r0, #0x0d \n"
                 "msr cpsr, r0 \n"
                 : : "r" (thread->arch.regs.sp), "r" (thread->arch.regs.lr));
}