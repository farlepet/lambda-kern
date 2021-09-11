#ifndef PROC_THREAD_H
#define PROC_THREAD_H

#include <proc/proc.h>

#include <stdint.h>

/**
 * \brief Create a new thread
 * 
 * New thread will not be attached to a process, and will require more setup
 * before it is ready to be scheduled.
 * 
 * @param entrypoint Execution entry point of thread
 * @param data Pointer to pass to thread function
 * @param name Name of thread
 * @param stack_size Size of user stack
 * @param prio Thread priority
 * 
 * @return NULL on error, else pointer to thread
 */
kthread_t *thread_create(uintptr_t entrypoint, void *data, const char *name, size_t stack_size, int prio);

/**
 * \brief Add a thread to the current process
 * 
 * @param entrypoint Entrypoint function of the thread
 * @param data Data to pass to thread function
 * @param name Optional name to give the thread
 * @param stack_size Desired thread stack size
 * @param prio Thread priority
 * 
 * @return < 0 on error, else TID of newly spawned thread
 */
int thread_spawn(uintptr_t entrypoint, void *data, const char *name, size_t stack_size, int prio);

#endif