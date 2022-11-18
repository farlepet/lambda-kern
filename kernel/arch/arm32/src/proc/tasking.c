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

    /* @todo Support both user (0x10) and system (0x1F), possibly along with
     * other present (IRQ,FIQ,SUP) modes. */
    asm volatile("cps #0x1F    \n"
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

    /* TODO: Support multiple modes apart from system */
    //thread->arch.regs.cpsr = 0x60000113;
    thread->arch.regs.cpsr = 0x600001D2;
    thread->arch.regs.spsr = 0x6000011F;
    thread->arch.regs.ksp  = thread->arch.stack_kern.begin;
    thread->arch.regs.usp  = thread->arch.stack_user.begin;
    thread->arch.regs.lr   = (uint32_t)exit;

    return 0;
}

int arch_setup_process(kproc_t __unused *thread) {
    return 0;
}

void arch_setup_user_stack(kthread_t *thread, int argc, char **argv, char **envp) {
    /* @todo */
    (void)thread;
    (void)argc;
    (void)argv;
    (void)envp;
}

void arch_multitasking_init(void) {
    /* @todo */
}

int arch_postfork_setup(const kthread_t *parent, kthread_t *child) {
    /* @todo */
    (void)parent;
    (void)child;

    return -1;
}

__hot __optimize_none
void do_task_switch(void) {
    kthread_t *thread = sched_get_curr_thread(0);

    if(thread->flags & KTHREAD_FLAG_RANONCE) {
        /* Save the CPSR, stack pointers, and link register of the current thread */
        register void *r0 asm("r0") = (void *)&thread->arch.gpregs;
        asm volatile("stm %4, {r1-r11}\n"
            
                     "mrs %0, cpsr\n" /* Kernel Program Status Register */
                     "str sp, %1  \n" /* Kernel stack pointer */
                     "str lr, %3  \n" /* Kernel LR */

                     "cps #0x1F   \n" /* Thread stack pointer */
                     "str sp, %2  \n"
                     "cps #0x12   \n" :
                     "=r"(thread->arch.regs.cpsr),
                     "=m"(thread->arch.regs.ksp),
                     "=m"(thread->arch.regs.usp),
                     "=m"(thread->arch.regs.lr) :
                     "r"(r0));

        uint32_t pc = get_pc();
        if(pc == 0xFFFFFFFF) {
            /* We just came here from the bx at the end of this function, so we
             * are ready to exit and continue executing on the new thread */
            return;
        }
        thread->arch.regs.pc = pc;

        kdebug(DEBUGSRC_PROC, ERR_ALL, "-TID: %02d | PC: %p | SP: [%p,%p] | CPSR: %08X | NAME: %s",
            thread->tid, ((uint32_t *)thread->arch.stack_kern.begin)[-1],
            thread->arch.regs.usp, thread->arch.regs.ksp,
            thread->arch.regs.spsr, thread->name);

#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
        if((thread->process->domain != PROC_DOMAIN_KERNEL) &&
           ((thread->arch.regs.usp > thread->arch.stack_user.begin) ||
            (thread->arch.regs.usp < (thread->arch.stack_user.begin - thread->arch.stack_user.size)))) {
            kpanic("User stack pointer out-of-bounds!");
        }
        if((thread->arch.regs.ksp > thread->arch.stack_kern.begin) ||
           (thread->arch.regs.ksp < (thread->arch.stack_kern.begin - thread->arch.stack_kern.size))) {
            kpanic("Kernel stack pointer out-of-bounds!");
        }
#endif /* CHECK_STRICTNESS */

        /* Get next thread to run. */
        thread = sched_next_process(0);
    }

    kdebug(DEBUGSRC_PROC, ERR_ALL, "+TID: %02d | PC: %p | SP: [%p,%p] | CPSR: %08X | NAME: %s",
           thread->tid, ((uint32_t *)thread->arch.stack_kern.begin)[-1],
           thread->arch.regs.usp, thread->arch.regs.ksp,
           thread->arch.regs.spsr, thread->name);

    thread->flags |= KTHREAD_FLAG_RANONCE;

    /* TODO: Using specific registers here is not desired, there is likely a
     * better way to go about this. As-is, this portion of code, and the above
     * conterpart, is somewhat fragile. */
    register void *r0  asm("r0")  = (void *)&thread->arch.gpregs;
    register void *r12 asm("r12") = (void *)thread->arch.regs.pc;
    asm volatile("cps #0x1F          \n"
                 "ldr sp, %3         \n"

                 "msr cpsr, %0       \n"
                 "ldr lr,   %1       \n"
                 "ldr sp,   %2       \n"

                 "ldm %5, {r1-r11}   \n"

                 "mov r0, #0xFFFFFFFF\n"
                 "bx  %4             \n" ::
                 "r"(thread->arch.regs.cpsr),
                 "m"(thread->arch.regs.lr),
                 "m"(thread->arch.regs.ksp),
                 "m"(thread->arch.regs.usp),
                 "r"(r12),
                 "r"(r0));
    
    __builtin_unreachable();
}