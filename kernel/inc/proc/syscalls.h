#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <types.h>

#include <arch/intr/int.h>

enum syscalls {
    SYSCALL_EXIT       = 3,

    SYSCALL_MMAP       = 4,
    SYSCALL_MUNMAP     = 5,

    SYSCALL_FS_ACCESS  = 9,
    SYSCALL_FS_READDIR = 10,
    SYSCALL_FS_STAT    = 11,
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
    SYSCALL_WAIT   = 23,

    SYSCALL_TASK_SWITCH = 24,

    SYSCALL_MAX
};


void syscalls_init(void);

void syscall_call(uint32_t scn, syscallarg_t *arg);
int syscall_service(uint32_t scn, syscallarg_t *args);

extern void return_from_syscall(void);

#endif
