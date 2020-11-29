#ifndef MTASK_H
#define MTASK_H

#include <arch/intr/int.h>

#include <types.h>
#include <proc/proc.h>

#define STACK_SIZE 0x8000 //!< Size of user stack or if kernel task has a unspecified stack size

//#define STACK_PROTECTOR //!< Whether or not to enable stack protectors (currently broken?)


extern struct kproc procs[MAX_PROCESSES];

extern int current_pid; //!< The PID of the currently running process

extern int tasking; //!< Whether or not multitasking has been started or not

int get_pid(); //!< Get the PID of the currently running task

/**
 * @brief Get the next free PID number
 * 
 * @return int Next PID, -1 if failure
 */
int get_next_pid();

/**
 * @brief Get the next free slot in process array.
 * 
 * @return int index in process array. < 0 on failure.
 */
int get_next_open_proc();

/**
 * @brief Initialize multitasking
 * 
 * Create process with given entrypoint, attach scheduler interrupt, and enable
 * multitasking.
 * 
 * @param process Entrypoint of initial kernel task
 * @param name Name of initial kernel task
 */
void init_multitasking(void *process, char *name);

/**
 * @brief Create and add a kernel task
 * 
 * @see add_task
 */
int add_kernel_task(void *process, char *name, uint32_t stack_size, int pri);

/**
 * @brief Create and add a kernel task, with a given page directory
 *
 * TODO: Remove x86-specific page directory reference
 *
 * @see add_task
 */
int add_kernel_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params);

/**
 * @brief Create and add a userland task
 * 
 * @see add_task
 */
int add_user_task(void *process, char *name, uint32_t stack_size, int pri);

/**
 * @brief Create and add a userland task, with a given page directory
 * 
 * TODO: Remove x86-specific page directory reference
 * 
 * @see add_task
 */
int add_user_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params);

/**
 * @brief Create and add a task
 * 
 * TODO: Remove x86-specific page directory and ring references
 * 
 * @param process Entrypoint of process
 * @param name Name of process
 * @param stack_size Size of stack to allocate to process
 * @param pri Priority level of process
 * @param pagedir Page directory of process
 * @param kernel Whether not process is a kernel process
 * @param ring Ring to run process in
 * @return int PID of process on success, else <=0
 */
int add_task(void *process, char* name, uint32_t stack_size, int pri, int kernel, arch_task_params_t *arch_params);

/**
 * @brief Get process index given PID
 * 
 * @param pid PID of process to find
 * @return int Index of process, else -1
 */
int proc_by_pid(int pid);

/**
 * @brief Stop calling process
 * 
 * @param code Exit code to return with
 */
__noreturn void exit(int code);

/**
 * @brief Fork process by duplicating process image
 * 
 * @return int Child: 0, Parent: child PID, -1 on error
 */
int fork(void);

/**
 * @brief Create stack for process
 * 
 * @param proc Process to create stack for
 * @param stack_size Size of stack to create
 * @param virt_stack_begin Virtual address at which to place stack
 * @param is_kernel Whether or not process is a kernel process
 * @return int 0 on success
 */
int proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel);

/**
 * @brief Create kernel stack for process
 * 
 * @param proc Process for which to create kernel stack
 * @return int 0 on success
 */
int proc_create_kernel_stack(struct kproc *proc);

/**
 * @brief Get pointer to current process
 * 
 * @return Pointer to current process, NULL on error
 */
struct kproc *mtask_get_current_task(void);


// TODO: Move x86-specific stack operation!
#define STACK_PUSH(esp, data) do { \
        esp = esp - 4; \
        *((uint32_t *)esp) = (uint32_t)data; \
    } while(0)

#endif
