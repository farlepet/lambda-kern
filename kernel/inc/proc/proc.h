#ifndef PROC_H
#define PROC_H

#include <mm/cbuff.h>
#include <fs/kfile.h>

#define MAX_PROCESSES        16 //!< Maximum amount of running processes
#define MAX_CHILDREN         8  //!< Maximum number of children a parent can handle
#define MAX_PROCESS_MESSAGES 64 //!< Maximum number of messages a process can retain
#define MAX_BLOCKED_PIDS     (MAX_PROCESSES - 1)
#define MAX_OPEN_FILES       8  //!< Maximum number of files opened by any particular process, including 0-2

#define MSG_BUFF_SIZE 512 //!< Size of the message buffer in bytes

#define TYPE_RUNNABLE 0x00000001 //!< Is this process runnable?
#define TYPE_RANONCE  0x00000002 //!< Can this process save its registers yet?
#define TYPE_VALID    0x00000004 //!< Is this a valid process? Can it be overwritten?
#define TYPE_ZOMBIE   0x00000008 //!< Has this task been killed?
#define TYPE_REAP     0x00000010 //!< Should this task be reaped?
#define TYPE_KERNEL   0x80000000 //!< Does this process run in kernel land?

#define BLOCK_DELAY       0x00000001 //!< Process is blocked waiting for a delay
#define BLOCK_MESSAGE     0x00000002 //!< Process is blocked waiting for a message
#define BLOCK_IPC_MESSAGE 0x00000004 //!< Process is blocked waiting for a message (mew IPC style)

#define PRIO_IDLE       0 //!< Only idle processes use this priority
#define PRIO_USERPROG   1 //!< Priority for user programs
#define PRIO_KERNELPROG 2 //!< Priority for kernel programs
#define PRIO_DRIVER     3 //!< Priority for kernel drivers
#define PRIO_KERNEL     4 //!< Priority for main kernel tasks

struct proc_book //!< Structure for process `book-keeping`
{
	u32 sent_msgs;   //!< Number of sent messages
	u32 sent_bytes;  //!< Number of sent bytes

	u32 recvd_msgs;  //!< Number of received messages
	u32 recvd_bytes; //!< Number of received bytes
};

struct kproc //!< Structure of a process as seen by the kernel
{
	char name[64]; //!< Name of the process
	int pid;       //!< Process ID
	int uid;       //!< User who `owns` the process
	int gid;       //!< Group who `owns` the process

	u32 type;      //!< Type of process

	int children[MAX_CHILDREN]; //!< Indexes of direct child processes (ex: NOT children's children)

#if  defined(ARCH_X86)
	u32 esp;       //!< Stack pointer
	u32 ebp;       //!< Stack base pointer
	u32 eip;       //!< Instruction pointer
	u32 cr3;       //!< Page directory

	u32 stack_beg; //!< Beginning of stack
	u32 stack_end; //!< Current end of stack
#endif

	struct cbuff messages;      //!< Message buffer structure
	u8 msg_buff[MSG_BUFF_SIZE]; //!< Actual buffer

	struct kfile *cwd; //!< Current working directory

	struct kfile *open_files[MAX_OPEN_FILES]; //!< Open file descriptors

	// New IPC:
	struct ipc_message *ipc_messages[MAX_PROCESS_MESSAGES]; //!< IPC message pointers
	int blocked_ipc_pids[MAX_BLOCKED_PIDS]; //!< PIDs blocked from sending messages to this process

	u32 blocked;   //!< Contains flags telling whether or not this process is blocked, and by what

	int exitcode;  //!< Exit code

	int prio;      //!< Task priority

	struct proc_book book; //!< Bookkeeping stuff
};


struct uproc //!< Structure of a process as seen by a user process
{
	char name[64]; //!< Name of the process
	int pid;       //!< Process ID
	int uid;       //!< User who `owns` the process
	int gid;       //!< Group who `owns` the process

	u32 type;      //!< Type of process

	int children[MAX_CHILDREN]; //!< Indexes of direct child processes (ex: NOT children's children)

	u32 ip;        //!< Instruction pointer

	u32 blocked;   //!< Contains flags telling whether or not this process is blocked, and by what

	int exitcode;  //!< Exit code

	int prio;      //!< Task priority
};


void kproc_to_uproc(struct kproc *kp, struct uproc *up);

/**
 * \brief Adds file to process
 * 
 * @param proc Process to add file to
 * @param file File to add
 * 
 * @return New file descriptor if applicable, -1 otherwise
 */
int proc_add_file(struct kproc *proc, struct kfile *file);

void sched_processes(void);

int sched_next_process(void);

#endif
