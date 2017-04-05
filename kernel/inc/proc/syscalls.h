#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <types.h>

#define SYSCALL_GET_KTASK 0
#define SYSCALL_SEND_MSG  1
#define SYSCALL_RECV_MSG  2
#define SYSCALL_EXIT      3

#define SYSCALL_INT       0xFF

void init_syscalls();

void call_syscall(u32 scn, u32 *arg);

#endif
