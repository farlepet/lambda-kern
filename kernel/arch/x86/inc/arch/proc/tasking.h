#ifndef ARCH_X86_PROC_TASKING_H
#define ARCH_X86_PROC_TASKING_H

#include <stdint.h>

#include <arch/intr/int.h>

typedef struct {
	arch_pusha_regs_t *pusha;
	arch_iret_regs_t  *iret;
} kproc_arch_syscall_regs_t;

/** Kernel representation of a stack allocated to a thread */
typedef struct {
    uint32_t begin;  /*!< Beginning of stack (i.e. top of allocated memory + 1) */
    uint32_t size;   /*!< Usable size of stack in bytes */
} arch_stack_t;

typedef struct {
	uint32_t esp;            /*!< Stack pointer */
	uint32_t ebp;            /*!< Stack base pointer */
	uint32_t eip;            /*!< Instruction pointer */

	arch_stack_t stack_kern; /*!< Kernel stack */
	arch_stack_t stack_user; /*!< User stack */

	uintptr_t stack_entry;   /*!< What to set ESP to on initial thread entry */

	kproc_arch_syscall_regs_t syscall_regs; //!< Syscall registers
} kthread_arch_t;

typedef struct {
	uint8_t  ring; /** Ring to run in (0-3) */
} kproc_arch_t;

/* Architecture-specific task creation parameters */
typedef struct {
    uint32_t *pgdir; //!< Page directory
	uint8_t   ring;  //!< Ring
} arch_task_params_t;

#include <intr/intr.h>
#include <proc/proc.h>

static inline void run_sched(void) {
	INTERRUPT(64);
}

/**
 * @brief Switch to next scheduled task
 * 
 * Switches context into next task, doesn't return within the same context
 */
void do_task_switch(void);

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



#endif
