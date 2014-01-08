#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <intr/int.h>

int send_message(int dest, void *msg, int size)
{
	// We don't want to be cut off doing this
	int en_ints = interrupts_enabled();
	disable_interrupts();

	int idx = proc_by_pid(dest);

	struct cbuff *buff = &(procs[idx].messages);

	u32 err = (u32)write_cbuff((u8 *)msg, size, buff);

	procs[idx].blocked &= (u32)~BLOCK_MESSAGE;

	if(en_ints) enable_interrupts();

	if(err & 0xFF000000)
	{
		kerror(ERR_SMERR, "send_message: couldn't send message to pid %d due to error", dest);
		return (int)err;
	}

	return 0;
}

int recv_message(void *msg, int size)
{
	int idx = proc_by_pid(current_pid);

	struct cbuff *buff = &(procs[idx].messages);

	u32 err;
	while((err = (u32)read_cbuff((u8 *)msg, size, buff)) & (CBUFF_EMPTY | CBUFF_NENOD))
	{
		procs[idx].blocked |= BLOCK_MESSAGE;
		busy_wait();
	}

	if(err & 0xFF000000)
	{
		kerror(ERR_SMERR, "recv_message: couldn't receive message due to error");
		return (int)err;
	}

	return 0;
}