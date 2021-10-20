#ifndef ARCH_ARMV7_PROC_TASKING_H
#define ARCH_ARMV7_PROC_TASKING_H

#include <stdint.h>

#include <err/panic.h>

#include <arch/intr/int.h>

typedef struct {
    volatile struct {
        uint32_t ksp;  /*!< Kernel stack pointer */
        uint32_t usp;  /*!< User stack pointer */
        uint32_t lr;   /*!< Link register */
        uint32_t pc;   /*!< Program counter */
        uint32_t cpsr; /*!< Current program status register */
        uint32_t spsr; /*!< Saved program status register - only used in _thread_entrypoint() */
    } regs;                     /*!< Saved registers */

	uint32_t kernel_stack;      /*!< Kernel stack */
	uint32_t kernel_stack_size; /*!< Size of kernel stack */

	uint32_t stack_beg;         /*!< Beginning of stack */
	uint32_t stack_end;         /*!< Current end of stack */
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
    if(!interrupts_enabled()) {
        kpanic("run_sched: Interrupts not enabled!");
    }
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
 * \brief Architecture-specific multitasking initialization.
 */
void arch_multitasking_init(void);

/**
 * @brief Switch to next scheduled task
 * 
 * Switches context into next task, doesn't return within the same context
 */
void do_task_switch(void);

#endif