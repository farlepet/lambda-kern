#ifndef PROC_THREAD_H
#define PROC_THREAD_H

#include <stdint.h>

/**
 * \brief Add a thread to the current process
 * 
 * @param entrypoint Entrypoint function of the thread
 * @param data Data to pass to thread function
 * @param name Optional name to give the thread
 * @param stack_size Desired thread stack size
 * @param prio Thread priority
 */
int kthread_create(void *entrypoint, void *data, const char *name, size_t stack_size, int prio);

#endif