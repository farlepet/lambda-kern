#ifndef MTASK_H
#define MTASK_H

#include <arch/intr/int.h>

#include <types.h>
#include <proc/proc.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/mm/paging.h>
#endif

extern lock_t creat_task; //!< Lock used when creating tasks

#define DEFAULT_STACK_SIZE 0x8000 //!< Size of user stack or if kernel task has a unspecified stack size

//#define STACK_PROTECTOR //!< Whether or not to enable stack protectors (currently broken?)

int get_pid(void); //!< Get the PID of the currently running task

/**
 * \brief Get the next free PID number
 * 
 * @return int Next PID, -1 if failure
 */
int get_next_pid(void);

/**
 * \brief Initialize multitasking
 * 
 * Create process with given entrypoint, attach scheduler interrupt, and enable
 * multitasking.
 * 
 * @param process Entrypoint of initial kernel task
 * @param name Name of initial kernel task
 */
void init_multitasking(void *process, char *name);



/**
 * \brief Create and add a task
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
 * \brief Get process pointer given PID
 * 
 * @param pid PID of process to find
 * @return Pointer to process if found, else NULL
 */
struct kproc *proc_by_pid(int pid);

/**
 * \brief Get thread pointer given TID
 * 
 * @param tid TID of thread to find
 * @return Pointer to thread if found, else NULL
 */
kthread_t *thread_by_tid(int tid);

/**
 * \brief Stop calling process
 * 
 * @param code Exit code to return with
 */
__noreturn void exit(int code);

/**
 * \brief Fork process by duplicating process image
 * 
 * @return int Child: 0, Parent: child PID, -1 on error
 */
int fork(void);

/**
 * \brief Create stack for process
 * 
 * @param thread Thread to create stack for
 * 
 * @return 0 on success, else non-zero
 */
int proc_create_stack(kthread_t *thread);

/**
 * \brief Create kernel stack for process
 * 
 * @param proc Process for which to create kernel stack
 * @return int 0 on success
 */
int proc_create_kernel_stack(kthread_t *thread);

/**
 * \brief Get current thread running on the CPU from which it was queried
 * 
 * @return NULL on error, else current thread
 */
kthread_t *mtask_get_curr_thread(void);

/**
 * \brief Get current process running on the CPU from which it was queried
 * 
 * @return Pointer to current process, NULL on error
 */
struct kproc *mtask_get_curr_process(void);

/**
 * \brief Insert process into process list.
 * 
 * @param proc Process to insert
 * @return 0 on success, else non-zero
 */
int mtask_insert_proc(struct kproc *proc);

// TODO: Move x86-specific stack operation!
#define STACK_PUSH(esp, data) do { \
        esp = esp - 4; \
        *((uint32_t *)esp) = (uint32_t)data; \
    } while(0)




/**
 * \brief Create and add a kernel task
 * 
 * @see add_task
 */
static inline int add_kernel_task(void *process, char *name, uint32_t stack_size, int pri) {
	arch_task_params_t arch_params;
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	arch_params.ring  = 0;
	arch_params.pgdir = clone_kpagedir();
#endif
	return add_task(process, name, stack_size, pri, 1, &arch_params);
}

/**
 * \brief Create and add a kernel task, with given architecture params
 *
 * @see add_task
 */
static inline int add_kernel_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params) {
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	arch_params->ring = 0;
#endif
	return add_task(process, name, stack_size, pri, 1, arch_params);
}

/**
 * \brief Create and add a userland task
 * 
 * @see add_task
 */
static inline int add_user_task(void *process, char *name, uint32_t stack_size, int pri) {
	arch_task_params_t arch_params;
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	arch_params.ring = 3;
	arch_params.pgdir = clone_kpagedir();
#endif
	return add_task(process, name, stack_size, pri, 0, &arch_params);
}

/**
 * \brief Create and add a userland task, with given architecture params
 * 
 * @see add_task
 */
static inline int add_user_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params) {
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	arch_params->ring = 3;
#endif
	return add_task(process, name, stack_size, pri, 0, arch_params);
}

#endif
