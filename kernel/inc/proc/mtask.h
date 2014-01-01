#ifndef MTASK_H
#define MTASK_H

#include <types.h>
#include <proc/proc.h>

struct kproc procs[MAX_PROCESSES];

int  current_pid; //!< The PID of the currently running process

int tasking; //!< Whether or not multitasking has been started or not

void *get_eip(); //!< Get the EIP value of the instruction after the call to this function

void init_multitasking(void *process, char *name);

void do_task_switch();

void add_kernel_task(void *process, char *name, u32 stack_size);

int proc_by_pid(int pid);

void exit(int code);

#define STACK_SIZE 0x8000 //!< Size of user stack or if kernel task has a unspecified stack size

//#define STACK_PROTECTOR //!< Whether or not to enable stack protectors

#define MULTITASKING_TEST 1 //!< Whether or not to spawn the multitasking testing processes

#endif