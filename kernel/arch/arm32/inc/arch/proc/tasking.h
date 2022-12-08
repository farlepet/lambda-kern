#ifndef ARCH_ARM32_PROC_TASKING_H
#define ARCH_ARM32_PROC_TASKING_H

#include <stdint.h>

#include <err/panic.h>

#include <arch/intr/types/intr.h>

/** Kernel representation of a stack allocated to a thread */
typedef struct {
    uint32_t begin;  /*!< Beginning of stack (i.e. top of allocated memory + 1) */
    uint32_t size;   /*!< Usable size of stack in bytes */
} arch_stack_t;

typedef struct {
    struct {
        uint32_t ksp;  /*!< Kernel stack pointer */
        uint32_t usp;  /*!< User stack pointer */
        uint32_t klr;  /*!< Kernel link register */
        uint32_t ulr;  /*!< User link register */
        uint32_t cpsr; /*!< Current program status register */
        uint32_t spsr; /*!< Saved program status register - only used in _thread_entrypoint() */
    } regs;                  /*!< Saved registers */
    
    struct {
        /* NOTE: We don't care about R0, since it's being used to check for
         * completion of context switch. */
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t r12;
    } gpregs; /*!< Saved general-purpose registers */

    arch_stack_t stack_kern; /*!< Kernel stack */
    arch_stack_t stack_user; /*!< User stack */

    arm32_intr_frame_t *int_frame; /**< Pointer to last interrupt frame - for debugging */
} kthread_arch_t;

typedef struct {
    int _dummy;
} kproc_arch_t;

/* Architecture-specific task creation parameters */
typedef struct {
    int _dummy;
} arch_task_params_t;

#include <proc/proc.h>

static inline void run_sched(void) {
    /* TODO: Force scheduler to run */
    /* @note There are some situations where interrupts may be disabled prior to
     * calling run_sched in order to ensure certain operations get completed
     * before the scheduler runs. */
    enable_interrupts();
    /* Wait for timer interrupt to fire: */
    interrupt_halt();
}

/**
 * \brief Architecture-specific process stack creation routine
 */
int arch_proc_create_stack(kthread_t *thread);

/**
 * \brief Architecture-specific process kernel stack creation routine
 */
int arch_proc_create_kernel_stack(kthread_t *thread);

/**
 * \brief Architecture-specific process creation routine
 */
int arch_setup_process(kproc_t *proc);

int arch_setup_thread(kthread_t *thread);

/**
 * @brief Setup user thread stack to contain arguments and environment
 *
 * @param thread Thread to setup
 * @param argc Argument count
 * @param argv Pointer to argument list
 * @param envp Pointer to environment list
 */
void arch_setup_user_stack(kthread_t *thread, int argc, char **argv, char **envp);

/**
 * \brief Architecture-specific multitasking initialization.
 */
void arch_multitasking_init(void);

/**
 * @brief Switch to next scheduled task
 * 
 * Switches context into next task, doesn't return within the same context
 */
void do_task_switch(void);

/**
 * @brief Architecture-specific thread setup after data is copied for a fork
 *
 * @param parent Parent thread
 * @param child Child thread
 */
int arch_postfork_setup(const kthread_t *parent, kthread_t *child);

#endif
