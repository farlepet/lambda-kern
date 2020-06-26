#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <types.h>

#include <intr/int.h>

enum syscalls {
    SYSCALL_GET_KTASK = 0,
    SYSCALL_SEND_MSG  = 1,
    SYSCALL_RECV_MSG  = 2,
    SYSCALL_EXIT      = 3,

    SYSCALL_IPC_SEND              = 4,
    SYSCALL_IPC_RECV              = 5,
    SYSCALL_IPC_RECV_PID          = 6,
    SYSCALL_IPC_RECV_BLOCKING     = 7,
    SYSCALL_IPC_RECV_PID_BLOCKING = 8,
    SYSCALL_IPC_COPY_MSG          = 9,
    SYSCALL_IPC_DELETE_MSG        = 10,
    SYSCALL_IPC_BLOCK_PID         = 11,

    SYSCALL_FS_READ    = 12,
    SYSCALL_FS_WRITE   = 13,
    SYSCALL_FS_OPEN    = 14,
    SYSCALL_FS_CLOSE   = 15,
    SYSCALL_FS_MKDIR   = 16,
    SYSCALL_FS_CREATE  = 17,
    SYSCALL_FS_IOCTL   = 18,

    SYSCALL_FS_READ_BLK   = 19,
    SYSCALL_FS_GETDIRINFO = 20,

    SYSCALL_FORK   = 21,
    SYSCALL_EXECVE = 22,
    SYSCALL_WAIT   = 23
};


#define SYSCALL_INT       0xFF

void init_syscalls();

void call_syscall(uint32_t scn, uint32_t *arg);
void handle_syscall(struct pusha_regs regs, struct iret_regs iregs);

extern void return_from_syscall();

#endif
