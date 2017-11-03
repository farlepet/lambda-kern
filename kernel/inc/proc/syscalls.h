#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <types.h>

#define SYSCALL_GET_KTASK    0
#define SYSCALL_SEND_MSG     1
#define SYSCALL_RECV_MSG     2
#define SYSCALL_EXIT         3

#define SYSCALL_IPC_SEND              4
#define SYSCALL_IPC_RECV              5
#define SYSCALL_IPC_RECV_PID          6
#define SYSCALL_IPC_RECV_BLOCKING     7
#define SYSCALL_IPC_RECV_PID_BLOCKING 8
#define SYSCALL_IPC_COPY_MSG          9
#define SYSCALL_IPC_DELETE_MSG        10
#define SYSCALL_IPC_BLOCK_PID         11

#define SYSCALL_FS_READ    12
#define SYSCALL_FS_WRITE   13
#define SYSCALL_FS_OPEN    14
#define SYSCALL_FS_CLOSE   15
#define SYSCALL_FS_READDIR 16
#define SYSCALL_FS_FINDDIR 17
#define SYSCALL_FS_MKDIR   18
#define SYSCALL_FS_CREATE  19
#define SYSCALL_FS_IOCTL   20

#define SYSCALL_FS_READ_BLK 21


#define SYSCALL_INT       0xFF

void init_syscalls();

void call_syscall(u32 scn, u32 *arg);

#endif
