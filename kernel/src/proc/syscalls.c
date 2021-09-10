// TODO: Abstraction:
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#include <arch/intr/idt.h>
#include <arch/proc/stack_trace.h>
#endif

#include <proc/syscalls.h>
#include <intr/intr.h>
#include <err/error.h>
#include <err/panic.h>
#include <string.h>

// Includes that include syscalls
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <proc/exec.h>
#include <fs/procfs.h>

#if 0
#  define syscall_debug(...) kdebug(__VA_ARGS__)
#else
#  define syscall_debug(...)
#endif

typedef struct {
	func0_t  func;   /** Pointer to function */
	int      nargs;  /** Number of arguments */
	uint64_t ncalls; /** Number of times this syscall was called */
} syscall_desc_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
static syscall_desc_t syscalls[] = {
	[SYSCALL_EXIT]      = { (func0_t)exit,         1, 0 },
	
	[SYSCALL_FS_READ]     = { (func0_t)proc_fs_read,     4, 0 },
	[SYSCALL_FS_WRITE]    = { (func0_t)proc_fs_write,    4, 0 },
	[SYSCALL_FS_OPEN]     = { (func0_t)proc_fs_open,     2, 0 },
	[SYSCALL_FS_CLOSE]    = { (func0_t)proc_fs_close,    1, 0 },
	[SYSCALL_FS_MKDIR]    = { (func0_t)proc_fs_mkdir,    3, 0 },
	[SYSCALL_FS_CREATE]   = { (func0_t)proc_fs_create,   3, 0 },
	[SYSCALL_FS_IOCTL]    = { (func0_t)proc_fs_ioctl,    3, 0 },

	[SYSCALL_FS_READ_BLK]   = { (func0_t)proc_fs_read_blk,   4, 0 },
	[SYSCALL_FS_GETDIRINFO] = { (func0_t)proc_fs_getdirinfo, 2, 0 },

	[SYSCALL_FORK]   = { (func0_t)fork,   0, 0 },
	[SYSCALL_EXECVE] = { (func0_t)execve, 3, 0 },
	[SYSCALL_WAIT]   = { (func0_t)wait,   1, 0 },

	[SYSCALL_TASK_SWITCH] = { (func0_t)do_task_switch, 0, 0 },
	
	[SYSCALL_FS_READDIR] = { (func0_t)proc_fs_readdir, 4, 0 }
};
#pragma GCC diagnostic pop

int service_syscall(uint32_t scn, syscallarg_t *args) {
	syscall_debug(DEBUGSRC_SYSCALL, "Syscall %d called with args at %08X", scn, args);
	if((scn >= ARRAY_SZ(syscalls)) ||
	   !syscalls[scn].func) {
		kproc_t *curr_proc = mtask_get_curr_process();
		kerror(ERR_MEDERR, "Process %d (%s) has tried to call an invalid syscall: %u Args: %08X", curr_proc->pid, curr_proc->name, scn, args);
		return -1;
	}

	syscalls[scn].ncalls++;

	func0_t func = syscalls[scn].func;

	// This could be made better, but it is more complicated if another architecture is supported
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
	switch(syscalls[scn].nargs) {
		case 0:
			args[0] = func();
			break;

		case 1:
			syscall_debug(DEBUGSRC_SYSCALL, "  -> ARGS: { %08X }", args[0]);
			args[0] = (syscallarg_t)((func1_t)func)(args[0]);
			break;

		case 2:
			syscall_debug(DEBUGSRC_SYSCALL, "  -> ARGS: { %08X %08X }", args[0], args[1]);
			args[0] = (syscallarg_t)((func2_t)func)(args[0], args[1]);
			break;

		case 3:
			syscall_debug(DEBUGSRC_SYSCALL, "  -> ARGS: { %08X %08X %08X }", args[0], args[1], args[2]);
			args[0] = (syscallarg_t)((func3_t)func)(args[0], args[1], args[2]);
			break;
		
		case 4:
			syscall_debug(DEBUGSRC_SYSCALL, "  -> ARGS: { %08X %08X %08X %08X }", args[0], args[1], args[2], args[3]);
			args[0] = (syscallarg_t)((func4_t)func)(args[0], args[1], args[2], args[3]);
			break;

		default:
			kpanic("Syscall error (%d): %d arguments not handled! Kernel programming error!", scn, syscalls[scn].nargs);
	}
#pragma GCC diagnostic pop

	syscall_debug(DEBUGSRC_SYSCALL, "  -> Retval [0] = %d", args[0]);
	
	return 0;
}


extern void syscall_int(void);
void init_syscalls(void)
{
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	set_idt(INTR_SYSCALL, 0x08, IDT_ATTR(1, 3, 0, int32), &syscall_int);
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARMV7)
	set_interrupt(INTR_SYSCALL, &syscall_int);
#endif
}

extern void call_syscall_int(uint32_t, syscallarg_t *);
void call_syscall(uint32_t scn, uint32_t *args)
{
	kdebug(DEBUGSRC_SYSCALL, "call_syscall: %d, %08X", scn, args);
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	call_syscall_int(scn, args);
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARMV7)
	(void)scn;
	(void)args;
	asm volatile("swi #1");
#endif
}
