#include <arch/proc/tasking.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <mm/alloc.h>

extern lock_t creat_task; // From proc/mtask.c

static int c_proc = 0; //!< Index of current process

extern uint32_t irq_stack_end[];


int arch_proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel) {
    (void)proc;
    (void)stack_size;
    (void)virt_stack_begin;
    (void)is_kernel;

    /* Might need to increase alignment when incorporating MMU */
    uint32_t stack = (uint32_t)kamalloc(stack_size, 16);
    if(!stack) {
        return -1;
    }

    /* TODO: MMU */
    proc->arch.stack_beg = stack;
    proc->arch.stack_end = stack + stack_size;

    /* TODO */
    return 0;
}

int arch_proc_create_kernel_stack(struct kproc __unused *proc) {
    /* NOTE: At least at present, kernel stack is shared by all processes. */
    return 0;
}

int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, int kernel, arch_task_params_t __unused *arch_params) {
    proc->arch.regs.pc = (uint32_t)entrypoint;
    proc->entrypoint   = (uint32_t)entrypoint;

    proc_create_stack(proc, stack_size, 0, kernel);

    proc->arch.regs.sp = proc->arch.stack_end;

    return 0;
}

void arch_multitasking_init(void) {
    /* TODO */
}

__hot void do_task_switch(void) {
	if(!tasking)   return;
	if(creat_task) return; /* We don't want to interrupt process creation */

    if(procs[c_proc].type & TYPE_RANONCE) {
        /* Save registers */
        /* push {lr}
         * push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
         * mrs  r0, spsr
         * push {r0} */
        /* TODO: Cleanup */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        procs[c_proc].arch.regs.pc   = irq_stack_end[-1];
        procs[c_proc].arch.regs.r0   = irq_stack_end[-2];
        procs[c_proc].arch.regs.r1   = irq_stack_end[-3];
        procs[c_proc].arch.regs.r2   = irq_stack_end[-4];
        procs[c_proc].arch.regs.r3   = irq_stack_end[-5];
        procs[c_proc].arch.regs.r4   = irq_stack_end[-6];
        procs[c_proc].arch.regs.r5   = irq_stack_end[-7];
        procs[c_proc].arch.regs.r6   = irq_stack_end[-8];
        procs[c_proc].arch.regs.r7   = irq_stack_end[-9];
        procs[c_proc].arch.regs.r8   = irq_stack_end[-10];
        procs[c_proc].arch.regs.r9   = irq_stack_end[-11];
        procs[c_proc].arch.regs.r10  = irq_stack_end[-12];
        procs[c_proc].arch.regs.r11  = irq_stack_end[-13];
        procs[c_proc].arch.regs.r12  = irq_stack_end[-14];
        procs[c_proc].arch.regs.cpsr = irq_stack_end[-15];
#pragma GCC diagnostic pop

        /* Save SP and LR: */
        asm("mrs r0, cpsr \n"
            "bic r0, r0, #0x1f \n"
            "orr r0, r0, #0x1f \n"
            "msr cpsr, r0 \n"
            "mov %0, sp \n"
            "mov %1, lr \n"
            "bic r0, r0, #0x1f \n"
            "orr r0, r0, #0x12 \n"
            "msr cpsr, r0 \n"
            : "=r" (procs[c_proc].arch.regs.sp), "=r" (procs[c_proc].arch.regs.lr));
    } else {
        procs[c_proc].type |= TYPE_RANONCE;
    }

    /* Get next process to run. */
    c_proc = sched_next_process();

    /* Load registers: */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    irq_stack_end[-1]  = procs[c_proc].arch.regs.pc;
    irq_stack_end[-2]  = procs[c_proc].arch.regs.r0;
    irq_stack_end[-3]  = procs[c_proc].arch.regs.r1;
    irq_stack_end[-4]  = procs[c_proc].arch.regs.r2;
    irq_stack_end[-5]  = procs[c_proc].arch.regs.r3;
    irq_stack_end[-6]  = procs[c_proc].arch.regs.r4;
    irq_stack_end[-7]  = procs[c_proc].arch.regs.r5;
    irq_stack_end[-8]  = procs[c_proc].arch.regs.r6;
    irq_stack_end[-9]  = procs[c_proc].arch.regs.r7;
    irq_stack_end[-10] = procs[c_proc].arch.regs.r8;
    irq_stack_end[-11] = procs[c_proc].arch.regs.r9;
    irq_stack_end[-12] = procs[c_proc].arch.regs.r10;
    irq_stack_end[-13] = procs[c_proc].arch.regs.r11;
    irq_stack_end[-14] = procs[c_proc].arch.regs.r12;
    irq_stack_end[-15] = procs[c_proc].arch.regs.cpsr;
#pragma GCC diagnostic pop

    /* Restore SP and LR: */
    asm("mrs r0, cpsr \n"
        "bic r0, r0, #0x1f \n"
        "orr r0, r0, #0x1f \n"
        "msr cpsr, r0 \n"
        "mov sp, %0 \n"
        "mov lr, %1 \n"
        "bic r0, r0, #0x1f \n"
        "orr r0, r0, #0x12 \n"
        "msr cpsr, r0 \n"
		: : "r" (procs[c_proc].arch.regs.sp), "r" (procs[c_proc].arch.regs.lr));
}