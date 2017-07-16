#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <mm/paging.h>
#include <proc/ipc.h>
#include <intr/int.h>
#include <mm/alloc.h>
#include <string.h>

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
		kerror(ERR_SMERR, "send_message: couldn't send message to pid %d due to error: 0x%X", dest, (err & 0xFF000000));
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

static uint32_t next_message_id = 0;

int ipc_create_user_message(struct ipc_message *msg, struct ipc_message_user *umsg)
{
	umsg->message_id = msg->message_id;
	umsg->src_pid    = msg->src_pid;
	umsg->length     = msg->length;

	return 0;
}

int ipc_create_message(struct ipc_message **msg, int src, int dest, void *message, uint32_t length)
{
	struct ipc_message *_msg = (struct ipc_message *)kmalloc(sizeof(struct ipc_message));
	if(*msg == NULL) return -1;
	*msg = _msg;


	_msg->message_id = next_message_id;
	next_message_id++;

	_msg->src_pid  = src;
	_msg->dest_pid = dest;

	_msg->message = kmalloc(length);
	if(_msg->message == NULL) return -2;

	_msg->length  = length;
	memcpy(_msg->message, message, length);

	return 0;
}

int ipc_delete_message(struct ipc_message *msg)
{
	// TODO: Delete message structure itself

	kfree(msg->message);

	return 0;
}

int ipc_copy_message_data(struct ipc_message *msg, void *dest)
{
	if(dest         == NULL) return -1;
	if(msg          == NULL) return -2;
	if(msg->message == NULL) return -3;

	memcpy(dest, msg->message, msg->length);

	return 0;
}

int ipc_send_message(struct ipc_message *msg)
{
	if(msg == NULL) return -1;

	int idx = proc_by_pid(msg->dest_pid);

	struct ipc_message **messages = procs[idx].ipc_messages;

	for(int i = 0; i < MAX_PROCESS_MESSAGES; i++)
	{
		if(messages[i] == NULL)
		{
			messages[i] = msg;
			procs[idx].blocked &= (u32)~BLOCK_IPC_MESSAGE;
			return 0;
		}
	}

	return -2; // Could not find empty slot
}



/*
 * User functions:
 */

int ipc_user_recv_message(struct ipc_message_user *umsg)
{
	if(umsg == NULL) return -2;

	int idx = proc_by_pid(current_pid);

	struct ipc_message **messages = procs[idx].ipc_messages;

	for(int i = 0; i < MAX_PROCESS_MESSAGES; i++)
	{
		if(messages[i] != NULL)
		{
			if(ipc_create_user_message(messages[i], umsg) < 0)
			{
				return -3; // Error while copying message data
			}
			return 0;
		}
	}

	return -1; // No waiting messages
}

int ipc_user_recv_message_pid(struct ipc_message_user *umsg, int pid)
{
	if(umsg == NULL) return -2;

	int idx = proc_by_pid(current_pid);

	struct ipc_message **messages = procs[idx].ipc_messages;

	for(int i = 0; i < MAX_PROCESS_MESSAGES; i++)
	{
		if(messages[i] != NULL)
		{
			if(messages[i]->src_pid == pid) {
				if(ipc_create_user_message(messages[i], umsg) < 0)
				{
					return -3; // Error while copying message data
				}
				return 0;
			}
		}
	}

	return -1; // No waiting messages
}

int ipc_user_recv_message_blocking(struct ipc_message_user *umsg)
{
	if(umsg == NULL) return -2;

	int idx = proc_by_pid(current_pid);

	int ret;
	while((ret = ipc_user_recv_message(umsg)) == -1)
	{
		procs[idx].blocked |= BLOCK_IPC_MESSAGE;
		busy_wait();
	}

	return ret;
}

int ipc_user_recv_message_pid_blocking(struct ipc_message_user *umsg, int pid)
{
	if(umsg == NULL) return -2;

	int idx = proc_by_pid(current_pid);

	int ret;
	while((ret = ipc_user_recv_message_pid(umsg, pid)) == -1)
	{
		procs[idx].blocked |= BLOCK_IPC_MESSAGE;
		busy_wait();
	}

	return ret;
}

int ipc_user_copy_message(uint32_t message_id, void *dest)
{
	if(dest == NULL) return -2;

	int idx = proc_by_pid(current_pid);

	struct ipc_message **messages = procs[idx].ipc_messages;

	for(int i = 0; i < MAX_PROCESS_MESSAGES; i++)
	{
		if(messages[i] != NULL)
		{
			if(messages[i]->message_id == message_id)
			{
				if(ipc_copy_message_data(messages[i], dest) < 0) return -3;
				if(ipc_delete_message(messages[i]) < 0) return -4;
				kfree(messages[i]);
				messages[i] = NULL;

				return 0;
			}
		}
	}

	return -1; // Message not found
}

int ipc_user_create_and_send_message(int dest_pid, void *message, uint32_t length)
{
	struct ipc_message *msg;

	if(ipc_create_message(&msg, current_pid, dest_pid, message, length) < 0) return -1;
	if(ipc_send_message(msg)) return -2;

	return 0;
}

int ipc_user_delete_message(uint32_t message_id)
{
	int idx = proc_by_pid(current_pid);

	struct ipc_message **messages = procs[idx].ipc_messages;

	for(int i = 0; i < MAX_PROCESS_MESSAGES; i++)
	{
		if(messages[i] != NULL)
		{
			if(messages[i]->message_id == message_id)
			{
				if(ipc_delete_message(messages[i]) < 0) return -2;
				kfree(messages[i]);
				messages[i] = NULL;

				return 0;
			}
		}
	}

	return -1; // Message not found
}