#include <proc/syscalls.h>
#include <intr/intr.h>
#include <err/error.h>
#include <err/panic.h>

// Includes that include syscalls
#include <proc/ktasks.h>
#include <proc/ipc.h>
#include <proc/mtask.h>
#include <fs/procfs.h>

struct syscall
{
	func0_t func;	// Pointer to function
	int nargs;		// Number of arguments
	u64 ncalls;		// Number of times this syscall was called
};

struct syscall syscalls[] =
{
	[SYSCALL_GET_KTASK] = { (func0_t)get_ktask,    2, 0 },
	[SYSCALL_SEND_MSG]  = { (func0_t)send_message, 3, 0 },
	[SYSCALL_RECV_MSG]  = { (func0_t)recv_message, 2, 0 },
	[SYSCALL_EXIT]      = { (func0_t)exit,         1, 0 },
	
	[SYSCALL_IPC_SEND]              = { (func0_t)ipc_user_create_and_send_message,   3, 0 },
	[SYSCALL_IPC_RECV]              = { (func0_t)ipc_user_recv_message,              1, 0 },
	[SYSCALL_IPC_RECV_PID]          = { (func0_t)ipc_user_recv_message_pid,          2, 0 },
	[SYSCALL_IPC_RECV_BLOCKING]     = { (func0_t)ipc_user_recv_message_blocking,     1, 0 },
	[SYSCALL_IPC_RECV_PID_BLOCKING] = { (func0_t)ipc_user_recv_message_pid_blocking, 2, 0 },
	[SYSCALL_IPC_COPY_MSG]          = { (func0_t)ipc_user_copy_message,              2, 0 },
	[SYSCALL_IPC_DELETE_MSG]        = { (func0_t)ipc_user_delete_message,            1, 0 },
	[SYSCALL_IPC_BLOCK_PID]         = { (func0_t)ipc_user_block_pid,                 1, 0 },

	[SYSCALL_FS_READ]    = { (func0_t)proc_fs_read,    4, 0 },
	[SYSCALL_FS_WRITE]   = { (func0_t)proc_fs_write,   4, 0 },
	[SYSCALL_FS_OPEN]    = { (func0_t)proc_fs_open,    2, 0 },
	[SYSCALL_FS_CLOSE]   = { (func0_t)proc_fs_close,   1, 0 },
	[SYSCALL_FS_READDIR] = { (func0_t)proc_fs_readdir, 2, 0 },
	[SYSCALL_FS_FINDDIR] = { (func0_t)proc_fs_finddir, 2, 0 },
	[SYSCALL_FS_MKDIR]   = { (func0_t)proc_fs_mkdir,   3, 0 },
	[SYSCALL_FS_CREATE]  = { (func0_t)proc_fs_create,  3, 0 },
	[SYSCALL_FS_IOCTL]   = { (func0_t)proc_fs_ioctl,   3, 0 }
};

void handle_syscall(u32 scn, u32 *args)
{
	//kerror(ERR_BOOTINFO, "Syscall %d called with args at %08X", scn, args);
	if(scn >= ARRAY_SZ(syscalls))
	{
		int pid = get_pid();
		kerror(ERR_MEDERR, "Process %d has tried to call an invalid syscall: %d", pid, scn);
		exit(1);
	}

	syscalls[scn].ncalls++;

	func0_t func = syscalls[scn].func;

	// This could be made better, but it is more complicated if another architecture is supported
	switch(syscalls[scn].nargs)
	{
		case 0:
			args[0] = func();
			break;

		case 1:
			args[0] = (u32)((func1_t)func(args[0]));
			break;

		case 2:
			args[0] = (u32)((func2_t)func(args[0], args[1]));
			break;

		case 3:
			args[0] = (u32)((func3_t)func(args[0], args[1], args[2]));
			break;
		
		case 4:
			args[0] = (u32)((func4_t)func(args[0], args[1], args[2], args[3]));
			break;

		default:
			kpanic("Syscall error (%d): %d arguments not handled! Kernel programming error!", scn, syscalls[scn].nargs);
			break;
	}

	//kerror(ERR_BOOTINFO, "  -> Retval [0] = %d", args[0]);
}


extern void syscall_int();
void init_syscalls()
{
	set_interrupt(SYSCALL_INT, &syscall_int);
}

extern void call_syscall_int();
void call_syscall(u32 scn, u32 *args)
{
	//kerror(ERR_BOOTINFO, "call_syscall: %d, %08X", scn, args);
#ifdef ARCH_X86
	call_syscall_int(scn, args);
#endif
}
