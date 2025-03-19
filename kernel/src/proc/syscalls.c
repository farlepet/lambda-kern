#include <errno.h>
#include <string.h>

#include <err/error.h>
#include <err/panic.h>
#include <intr/intr.h>
#include <proc/syscalls.h>

#include <arch/intr/syscall.h>

// Includes that include syscalls
#include <fs/procfs.h>
#include <mm/mmap.h>
#include <proc/exec.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>

#if 1
#  define syscall_debug(...) kdebug(DEBUGSRC_SYSCALL, __VA_ARGS__)
#else
#  define syscall_debug(...)
#endif

typedef struct {
    func0_t  func;   /** Pointer to function */
    int      nargs;  /** Number of arguments */
} syscall_desc_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
static const syscall_desc_t _syscalls[] = {
    [SYSCALL_EXIT]      = { (func0_t)exit,         1 },
    
    [SYSCALL_MMAP]      = { (func0_t)mmap,         6 },
    [SYSCALL_MUNMAP]    = { (func0_t)munmap,       2 },

    [SYSCALL_FS_ACCESS]   = { (func0_t)proc_fs_access,   4 },
    [SYSCALL_FS_READDIR]  = { (func0_t)proc_fs_readdir,  4 },
    [SYSCALL_FS_STAT]     = { (func0_t)proc_fs_stat,     3 },
    [SYSCALL_FS_READ]     = { (func0_t)proc_fs_read,     4 },
    [SYSCALL_FS_WRITE]    = { (func0_t)proc_fs_write,    4 },
    [SYSCALL_FS_OPEN]     = { (func0_t)proc_fs_open,     2 },
    [SYSCALL_FS_CLOSE]    = { (func0_t)proc_fs_close,    1 },
    [SYSCALL_FS_MKDIR]    = { (func0_t)proc_fs_mkdir,    3 },
    [SYSCALL_FS_CREATE]   = { (func0_t)proc_fs_create,   3 },
    [SYSCALL_FS_IOCTL]    = { (func0_t)proc_fs_ioctl,    3 },

    [SYSCALL_FS_READ_BLK]   = { (func0_t)proc_fs_read_blk,   4 },
    [SYSCALL_FS_GETDIRINFO] = { (func0_t)proc_fs_getdirinfo, 2 },

    [SYSCALL_FORK]   = { (func0_t)fork,   0 },
    [SYSCALL_EXECVE] = { (func0_t)execve, 3 },
    [SYSCALL_WAIT]   = { (func0_t)wait,   1 },

    [SYSCALL_TASK_SWITCH] = { (func0_t)do_task_switch, 0 },
};
#pragma GCC diagnostic pop

static const char *_syscall_stringify(uint32_t);

int syscall_service(uint32_t scn, syscallarg_t *args) {
    kthread_t *curr_thread = mtask_get_curr_thread();
    
    syscall_debug(ERR_TRACE, "Syscall %d [%s] called by %d with args at %08X", scn, _syscall_stringify(scn), curr_thread->tid, args);
    if((scn >= ARRAY_SZ(_syscalls)) ||
       !_syscalls[scn].func) {
        kdebug(DEBUGSRC_SYSCALL, ERR_INFO, "Thread %d (%s) has tried to call an invalid syscall: %u Args: %08X", curr_thread->tid, curr_thread->name, scn, args);
        return -ENOSYS;
    }

    func0_t func = _syscalls[scn].func;

    // This could be made better, but it is more complicated if another architecture is supported
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    switch(_syscalls[scn].nargs) {
        case 0:
            args[0] = func();
            break;

        case 1:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p }", args[0]);
            args[0] = (syscallarg_t)((func1_t)func)(args[0]);
            break;

        case 2:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p %p }", args[0], args[1]);
            args[0] = (syscallarg_t)((func2_t)func)(args[0], args[1]);
            break;

        case 3:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p %p %p }", args[0], args[1], args[2]);
            args[0] = (syscallarg_t)((func3_t)func)(args[0], args[1], args[2]);
            break;

        case 4:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p %p %p %p }", args[0], args[1], args[2], args[3]);
            args[0] = (syscallarg_t)((func4_t)func)(args[0], args[1], args[2], args[3]);
            break;

        case 5:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p %p %p %p %p }", args[0], args[1], args[2], args[3], args[4]);
            args[0] = (syscallarg_t)((func5_t)func)(args[0], args[1], args[2], args[3], args[4]);
            break;

        case 6:
            syscall_debug(ERR_TRACE, "  -> ARGS: { %p %p %p %p %p %p }", args[0], args[1], args[2], args[3], args[4], args[5]);
            args[0] = (syscallarg_t)((func6_t)func)(args[0], args[1], args[2], args[3], args[4], args[5]);
            break;

        default:
            kpanic("Syscall error (%d): %d arguments not handled! Kernel programming error!", scn, _syscalls[scn].nargs);
    }
#pragma GCC diagnostic pop

    syscall_debug(ERR_TRACE, "  -> Retval [0] = %d", args[0]);
    
    return 0;
}


static intr_handler_hand_t _syscall_int_hdlr = {
    .callback = arch_syscall_interrupt,
    .data     = NULL
};

void syscalls_init(void) {
    interrupt_attach(INTR_SYSCALL, &_syscall_int_hdlr);
}

void syscall_call(uint32_t scn, syscallarg_t *args) {
    kdebug(DEBUGSRC_SYSCALL, ERR_TRACE, "call_syscall: %d, %08X", scn, args);
    arch_call_syscall(scn, args);
}

static const char *_syscall_stringify(uint32_t scn) {
    return (scn == SYSCALL_EXIT)          ? "EXIT"        :
           (scn == SYSCALL_MMAP)          ? "MMAP"        :
           (scn == SYSCALL_MUNMAP)        ? "MUNMAP"      :
           (scn == SYSCALL_FS_READDIR)    ? "READDIR"     :
           (scn == SYSCALL_FS_ACCESS)     ? "ACCESS"      :
           (scn == SYSCALL_FS_STAT)       ? "STAT"        :
           (scn == SYSCALL_FS_READ)       ? "READ"        :
           (scn == SYSCALL_FS_WRITE)      ? "WRITE"       :
           (scn == SYSCALL_FS_OPEN)       ? "OPEN"        :
           (scn == SYSCALL_FS_CLOSE)      ? "CLOSE"       :
           (scn == SYSCALL_FS_MKDIR)      ? "MKDIR"       :
           (scn == SYSCALL_FS_CREATE)     ? "CREATE"      :
           (scn == SYSCALL_FS_IOCTL)      ? "IOCTL"       :
           (scn == SYSCALL_FS_READ_BLK)   ? "READ_BLK"    :
           (scn == SYSCALL_FS_GETDIRINFO) ? "GETDIRINFO"  :
           (scn == SYSCALL_FORK)          ? "FORK"        :
           (scn == SYSCALL_EXECVE)        ? "EXECVE"      :
           (scn == SYSCALL_WAIT)          ? "WAIT"        :
           (scn == SYSCALL_TASK_SWITCH)   ? "TASK_SWITCH" : "UNKNOWN";
}

