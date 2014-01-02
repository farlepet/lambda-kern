#ifndef PROC_H
#define PROC_H

#include "mtask.h"
#include <mm/cbuff.h>

#define MAX_PROCESSES 64 //!< Maximum amount of running processes
#define MAX_CHILDREN  32 //!< Maximum number of children a parent can take care of

#define MSG_BUFF_SIZE 512 //!< Size of the message buffer in bytes

#define TYPE_RUNNABLE 0x00000001 //!< Is this process runnable?
#define TYPE_RANONCE  0x00000002 //!< Can this process save its registers yet?
#define TYPE_VALID    0x00000004 //!< Is this a valid process? Can it be overwritten?
#define TYPE_ZOMBIE   0x00000008 //!< Has this task been killed?
#define TYPE_REAP     0x00000010 //!< Should this task be reaped?
#define TYPE_KERNEL   0x80000000 //!< Does this process run in kernel land?

#define BLOCK_DELAY   0x00000001 //!< Process is blocked waiting for a delay
#define BLOCK_MESSAGE 0x00000002 //!< Process is blocked waiting for a message


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

	u32 blocked;   //!< Contains flags telling whether or not this process is blocked, and by what

	int exitcode;  //!< Exit code
};

#endif