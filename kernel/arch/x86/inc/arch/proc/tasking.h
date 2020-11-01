#ifndef ARCH_X86_PROC_TASKING_H
#define ARCH_X86_PROC_TASKING_H

#include <stdint.h>

typedef struct {
	int ring;           //!< Ring to run in (0-3)

	uint32_t esp;       //!< Stack pointer
	uint32_t ebp;       //!< Stack base pointer
	uint32_t eip;       //!< Instruction pointer
	uint32_t cr3;       //!< Page directory

	uint32_t kernel_stack;      //!< Kernel stack
	uint32_t kernel_stack_size; //!< Size of kernel stack

	uint32_t stack_beg; //!< Beginning of stack
	uint32_t stack_end; //!< Current end of stack
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
int arch_proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel);

/**
 * \brief Architecture-specific process kernel stack creation routine
 */
int arch_proc_create_kernel_stack(struct kproc *proc);

/**
 * \brief Architecture-specific process creation routine
 */
int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, int kernel, arch_task_params_t *arch_params);

/**
 * \brief Architecture-specific multitasking initialization.
 */
void arch_multitasking_init(void);



#endif