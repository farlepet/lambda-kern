#ifndef ARCH_ARMV7_PROC_TASKING_H
#define ARCH_ARMV7_PROC_TASKING_H

#include <stdint.h>

typedef struct {
    struct {
        uint32_t r0;
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
        uint32_t sp;
        uint32_t lr;
        uint32_t cpsr;
        uint32_t pc;
    } regs;                     //!< Saved registers

	uint32_t kernel_stack;      //!< Kernel stack
	uint32_t kernel_stack_size; //!< Size of kernel stack

	uint32_t stack_beg;         //!< Beginning of stack
	uint32_t stack_end;         //!< Current end of stack
} kproc_arch_t;

/* Architecture-specific task creation parameters */
typedef struct {

} arch_task_params_t;

#include <proc/proc.h>

static inline void run_sched(void) {
	/* TODO */
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
int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, int kernel, arch_task_params_t *arch_params);

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