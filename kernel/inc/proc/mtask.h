#ifndef MTASK_H
#define MTASK_H

#include <types.h>

u32 current_pid; //!< The PID of the currently running process

void *get_eip(); //!< Get the EIP value of the instruction after the call to this function

#endif