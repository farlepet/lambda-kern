#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <intr/int.h>

int send_message(int dest, u8 *msg, int size)
{
	// We don't want to be cut off doing this
	int en_ints = interrupts_enabled();
	disable_interrupts();

	int idx = proc_by_pid(dest);

	struct cbuff *buff = &(procs[idx].messages);

	int err = write_cbuff(msg, size, buff);

	if(en_ints) enable_interrupts();

	if(err & 0xFF000000)
	{
		kerror(ERR_SMERR, "send_message: couldn't send message to pid %d due to error", dest);
		return err;
	}

	return 0;
}

int recv_message(u8 *msg, int size)
{
	int idx = proc_by_pid(current_pid);

	struct cbuff *buff = &(procs[idx].messages);

	int err;
	while((err = read_cbuff(msg, size, buff)) & (CBUFF_EMPTY | CBUFF_NENOD))
	{
		busy_wait();
	}

	if(err & 0xFF000000)
	{
		kerror(ERR_SMERR, "recv_message: couldn't receive message due to error");
		return err;
	}

	return 0;
}