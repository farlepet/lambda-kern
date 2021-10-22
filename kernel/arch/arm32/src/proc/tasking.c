#include <arch/proc/tasking.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>

extern lock_t creat_task; // From proc/mtask.c

extern uint32_t irq_stack_end[];

extern uintptr_t get_pc(void);

static int _allocate_stack(arch_stack_t *stack, uint32_t size) {
    stack->size  = size;
    stack->begin = (uint32_t)kmamalloc(size, 4096);
    if(!stack->begin) {
        return -1;
    }
    /* TODO: MMU */
    stack->begin += size;

    return 0;
}

int arch_proc_create_stack(kthread_t *thread) {
    if(_allocate_stack(&thread->arch.stack_user, thread->stack_size)) {
        return -1;
    }

    return 0;
}

int arch_proc_create_kernel_stack(kthread_t *thread) {
    if(_allocate_stack(&thread->arch.stack_kern, PROC_KERN_STACK_SIZE)) {
        return -1;
    }

    return 0;
}

__noreturn
static void _thread_entrypoint(void) {
	kthread_t *curr_thread = sched_get_curr_thread(0);
	if(curr_thread == NULL) {
		kpanic("_proc_entrypoint: Thread is NULL!");
	}

    asm volatile("cps #0x13    \n"
                 "mov sp,   %0 \n" /* Set user stack pointer */
                 "cps #0x12    \n"
                 
                 "mov sp,   %1 \n"
                 "msr spsr, %2 \n"
                 "push     {%3}\n"

                 "ldm sp!, {pc}^\n" ::
                 "r"(curr_thread->arch.regs.usp),
                 "r"(curr_thread->arch.regs.ksp),
                 "r"(curr_thread->arch.regs.spsr),
                 "r"(curr_thread->entrypoint));
    
    __builtin_unreachable();
}

int arch_setup_thread(kthread_t *thread) {
    thread->arch.regs.pc = (uint32_t)_thread_entrypoint;

    proc_create_stack(thread);
    proc_create_kernel_stack(thread);

    /* TODO: Support multiple modes apart from supervisor */
    //thread->arch.regs.cpsr = 0x60000113;
    thread->arch.regs.cpsr = 0x600001D2;
    thread->arch.regs.spsr = 0x60000113;
    thread->arch.regs.ksp  = thread->arch.stack_kern.begin;
    thread->arch.regs.usp  = thread->arch.stack_user.begin;
    thread->arch.regs.lr   = (uint32_t)exit;

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
        asm volatile("mrs %0, cpsr\n"
                     "mov %1, sp  \n"
                     "mov %3, lr  \n"

                     "cps #0x13   \n"
                     "mov %2, sp  \n"
                     "cps #0x12   \n" :
                     "=r"(thread->arch.regs.cpsr),
                     "=r"(thread->arch.regs.ksp),
                     "=r"(thread->arch.regs.usp),
                     "=r"(thread->arch.regs.lr));
        uint32_t pc = get_pc();
        if(pc == 0xFFFFFFFF) {
            return;
        }
        thread->arch.regs.pc = pc;
    }
    
    kdebug(DEBUGSRC_PROC, ERR_ALL, "-TID: %02d | PC: %08X | SP: %08X | CPSR: %08X | NAME: %s",
           thread->tid, ((uint32_t *)thread->arch.stack_kern.begin)[-1], thread->arch.regs.usp,
           thread->arch.regs.spsr, thread->name);

    /* Get next thread to run. */
    thread = sched_next_process(0);

    //kdebug(DEBUGSRC_PROC, ERR_ALL, "+TID: %d | PC: %08X | LR: %08X | SP: [%08X,%08X] | CPSR: %08X | NAME: %s", thread->tid, thread->arch.regs.pc, thread->arch.regs.lr, thread->arch.regs.ksp, thread->arch.regs.usp, thread->arch.regs.cpsr, thread->name);
    kdebug(DEBUGSRC_PROC, ERR_ALL, "+TID: %02d | PC: %08X | SP: %08X | CPSR: %08X | NAME: %s",
           thread->tid, ((uint32_t *)thread->arch.stack_kern.begin)[-1], thread->arch.regs.usp,
           thread->arch.regs.spsr, thread->name);

    thread->flags |= KTHREAD_FLAG_RANONCE;

    asm volatile("cps #0x13   \n"
                 "mov sp, %3  \n"
                 
                 "msr cpsr, %0       \n"
                 "mov lr,   %1       \n"
                 "mov sp,   %2       \n"
                 "mov r0, #0xFFFFFFFF\n"
                 "bx  %4             \n" ::
                 "r"(thread->arch.regs.cpsr),
                 "r"(thread->arch.regs.lr),
                 "r"(thread->arch.regs.ksp),
                 "r"(thread->arch.regs.usp),
                 "r"(thread->arch.regs.pc) :
                 "r0","lr");
}