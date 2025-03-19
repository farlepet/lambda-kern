#include <errno.h>

#include <arch/proc/tasking.h>

#include <lambda/config_defs.h>
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
        return -ENOMEM;
    }
    /* TODO: MMU */
    stack->begin += size;

    return 0;
}

int arch_proc_create_stack(kthread_t *thread) {
    TRY_OR_RET(_allocate_stack(&thread->arch.stack_user, thread->stack_size));

    return 0;
}

int arch_proc_create_kernel_stack(kthread_t *thread) {
    TRY_OR_RET(_allocate_stack(&thread->arch.stack_kern, CONFIG_PROC_KERN_STACK_SIZE));

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
    proc_create_stack(thread);
    proc_create_kernel_stack(thread);

    /* TODO: Support multiple modes apart from system */
    //thread->arch.regs.cpsr = 0x60000113;
    thread->arch.regs.cpsr = 0x600001D2;
    thread->arch.regs.spsr = 0x6000011F;
    thread->arch.regs.ksp  = thread->arch.stack_kern.begin;
    thread->arch.regs.usp  = thread->arch.stack_user.begin;
    thread->arch.regs.klr  = (uint32_t)_thread_entrypoint;
    thread->arch.regs.ulr  = (uint32_t)exit;

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

    return -EUNSPEC;
}

__naked
static int _thread_save(kthread_arch_t *arch __unused) {
    asm volatile("add r12, r0, %0\n"
                 "stm r12, {r1-r11}\n" /* Kernel R1-R11 */

                 "mrs r12, cpsr\n"     /* Kernel CPSR */
                 "str r12, [r0, %1]\n"
                 "str sp,  [r0, %2]\n" /* Kernel stack pointer */
                 "str lr,  [r0, %3]\n" /* Kernel link register */

                 "cps #0x1F\n"
                 "str sp, [r0, %4]\n"  /* Thread stack pointer */
                 "str lr, [r0, %5]\n"  /* Thread link register */
                 "msr cpsr, r12\n"

                 "mov r0, #0\n"
                 "bx lr\n"
                 ::
                 "i"(offsetof(kthread_arch_t, gpregs)),
                 "i"(offsetof(kthread_arch_t, regs.cpsr)),
                 "i"(offsetof(kthread_arch_t, regs.ksp)),
                 "i"(offsetof(kthread_arch_t, regs.klr)),
                 "i"(offsetof(kthread_arch_t, regs.usp)),
                 "i"(offsetof(kthread_arch_t, regs.ulr)));
}

__naked
static void _thread_load(kthread_arch_t *arch __unused) {
    asm volatile("ldr r12, [r0, %2]\n"

                 "cps #0x1F\n"
                 "ldr sp, [r0, %0]\n"  /* Thread stack pointer */
                 "ldr lr, [r0, %1]\n"  /* Thread link register */

                 "msr cpsr, r12\n"     /* Kernel CPSR */

                 "ldr sp, [r0, %3]\n"  /* Kernel stack pointer */
                 "ldr lr, [r0, %4]\n"  /* Kernel link register */

                 "add r12, r0, %5\n"
                 "ldm r12, {r1-r11}\n" /* Kernel R1-R11 */

                 "mov r0, #1\n"        /* Return 1 from _thread_save */
                 "bx lr\n"
                 ::
                 "i"(offsetof(kthread_arch_t, regs.usp)),
                 "i"(offsetof(kthread_arch_t, regs.ulr)),
                 "i"(offsetof(kthread_arch_t, regs.cpsr)),
                 "i"(offsetof(kthread_arch_t, regs.ksp)),
                 "i"(offsetof(kthread_arch_t, regs.klr)),
                 "i"(offsetof(kthread_arch_t, gpregs)));
}

__hot __optimize_none
void do_task_switch(void) {
    kthread_t *thread = sched_get_curr_thread(0);

    if(thread->flags & KTHREAD_FLAG_RANONCE) {
        if(_thread_save(&thread->arch)) {
            return;
        }

        kdebug(DEBUGSRC_PROC, ERR_ALL, "-TID: %02d | PC: %p | LR: [%p,%p] | SP: [%p,%p] | CPSR: %08x | NAME: %s",
            thread->tid, (thread->arch.int_frame ? thread->arch.int_frame->lr : 0),
            thread->arch.regs.ulr, thread->arch.regs.klr,
            thread->arch.regs.usp, thread->arch.regs.ksp,
            thread->arch.regs.spsr, thread->name);

#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
        if(((thread->arch.regs.usp > thread->arch.stack_user.begin) ||
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

    kdebug(DEBUGSRC_PROC, ERR_ALL, "+TID: %02d | PC: %p | LR: [%p,%p] | SP: [%p,%p] | CPSR: %08x | NAME: %s",
           thread->tid, (thread->arch.int_frame ? thread->arch.int_frame->lr : 0),
            thread->arch.regs.ulr, thread->arch.regs.klr,
           thread->arch.regs.usp, thread->arch.regs.ksp,
           thread->arch.regs.spsr, thread->name);

    thread->flags |= KTHREAD_FLAG_RANONCE;

    _thread_load(&thread->arch);
    
    __builtin_unreachable();
}

