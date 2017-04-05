#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <mm/paging.h>
#include <proc/ipc.h>
#include <intr/int.h>

lock_t send_lock = 0; //!< Make sure only 1 message is sent at a time

int send_message(int dest, void *msg, int size)
{
	// We don't want to be cut off doing this
	lock(&send_lock);
	
	// Get physical location of the message:
	//msg = get_phys_page(msg);
	//kerror(ERR_BOOTINFO, "Sending message of size %d from 0x%08X to %d", size, msg, dest);

	int idx = proc_by_pid(dest);

	struct cbuff *buff = &(procs[idx].messages);

	u32 err = (u32)write_cbuff((u8 *)msg, size, buff);

	procs[idx].blocked &= (u32)~BLOCK_MESSAGE;

	unlock(&send_lock);

	if(err & 0xFF000000)
	{
		kerror(ERR_SMERR, "send_message: couldn't send message to pid %d due to error", dest);
		return (int)err;
	}

	procs[idx].book.sent_msgs++;
	procs[idx].book.sent_bytes += (u32)size;

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

	procs[idx].book.recvd_msgs++;
	procs[idx].book.recvd_bytes += (u32)size;

	return 0;
}