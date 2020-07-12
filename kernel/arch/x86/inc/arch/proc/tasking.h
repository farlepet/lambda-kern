#ifndef ARCH_X86_PROC_TASKING_H
#define ARCH_X86_PROC_TASKING_H

#include <intr/intr.h>
#include <proc/proc.h>

inline void run_sched(void) {
	INTERRUPT(SCHED_INT);
}

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
int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, uint32_t *pagedir, int kernel, int ring);

/**
 * \brief Architecture-specific multitasking initialization.
 */
void arch_multitasking_init(void);

#endif