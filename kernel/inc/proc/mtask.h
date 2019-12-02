#ifndef MTASK_H
#define MTASK_H

void run_sched(void);

#include <types.h>
#include <proc/proc.h>

extern struct kproc procs[MAX_PROCESSES];

int  current_pid; //!< The PID of the currently running process

int tasking; //!< Whether or not multitasking has been started or not

void *get_eip(); //!< Get the EIP value of the instruction after the call to this function

int get_pid(); //!< Get the PID of the currently running task

void init_multitasking(void *process, char *name);

void do_task_switch(void);

int add_kernel_task(void *process, char *name, u32 stack_size, int pri);
int add_kernel_task_pdir(void *process, char *name, u32 stack_size, int pri, u32 *pagedir);
int add_user_task_pdir(void *process, char *name, u32 stack_size, int pri, u32 *pagedir);

int add_task(void *process, char* name, uint32_t stack_size, int pri, uint32_t *pagedir, int kernel, int ring);

int proc_by_pid(int pid);

void exit(int code);

int fork(void);

#define STACK_SIZE 0x8000 //!< Size of user stack or if kernel task has a unspecified stack size

//#define STACK_PROTECTOR //!< Whether or not to enable stack protectors (currently broken)

#define STACK_PUSH(esp, data) do { \
        esp = esp - 4; \
        *((uint32_t *)esp) = (uint32_t)data; \
    } while(0);

#endif
