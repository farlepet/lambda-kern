#ifndef ARCH_X86_PROC_STACK_H
#define ARCH_X86_PROC_STACK_H

#include <proc/proc.h>

/**
 * @brief Copies stack of calling process to that of the child process.
 * 
 * Note: The source process is assumed to be the current process
 * 
 * @param dest Destination process
 * @param src Source process
 * @return int 0 on success
 */
int proc_copy_stack(struct kproc *dest, const struct kproc *src);

/**
 * @brief Copies the kernel stack to that of the child process.
 * 
 * Note: The source process is assumed to be the current process
 * 
 * @param dest Destination process
 * @param src Source process
 * @return int 0 on success
 */
int proc_copy_kernel_stack(struct kproc *dest, const struct kproc *src);

#endif // ARCH_X86_PROC_STACK_H