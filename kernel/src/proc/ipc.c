#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <mm/paging.h>
#include <proc/ipc.h>
#include <intr/int.h>
#include <mm/alloc.h>
#include <string.h>

//static uint32_t next_message_id = 0;

static lock_t send_lock = 0; //!< Make sure only 1 message is sent at a time

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

/*********************
 * New IPC functions *
 *********************/

struct ipc_message *ipc_messages[IPC_MAX_MESSAGES];

int ipc_create_user_message(struct ipc_message *msg, struct ipc_message_user *umsg)
{
	umsg->message_id = msg->message_id;
	umsg->src_pid    = msg->src_pid;
	umsg->length     = msg->length;

	return 0;
}

int ipc_create_message(struct ipc_message *msg, int src, int dest, void *message, uint32_t length)
{
	// TODO: Set message ID

	msg->src_pid  = src;
	msg->dest_pid = dest;

	msg->message = kmalloc(length);
	if(msg->message == NULL) return -1;

	msg->length  = length;
	memcpy(msg->message, message, length);

	return 0;
}

int ipc_delete_message(struct ipc_message *msg)
{
	// TODO: Delete message structure itself

	kfree(msg->message);

	return 0;
}

int ipc_copy_message_data(struct ipc_message *msg, void *dest) {
	if(dest         == NULL) return -1;
	if(msg          == NULL) return -2;
	if(msg->message == NULL) return -3;

	memcpy(dest, msg->message, msg->length);

	return 0;
}