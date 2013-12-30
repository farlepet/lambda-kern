#ifndef MTASK_H
#define MTASK_H

#include <types.h>

int  current_pid; //!< The PID of the currently running process

void *get_eip(); //!< Get the EIP value of the instruction after the call to this function

void init_multitasking(void *process, char *name);

void do_task_switch();

void add_kernel_task(void *process, char *name);

#define STACK_SIZE 0x1000

#endif