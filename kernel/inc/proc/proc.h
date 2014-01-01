#ifndef PROC_H
#define PROC_H

#include "mtask.h"

#define MAX_PROCESSES 64 //!< Maximum amount of running processes
#define MAX_CHILDREN  32 //!< Maximum number of children a parent can take care of

#define TYPE_RUNNABLE 0x00000001 //!< Is this process runnable?
#define TYPE_RANONCE  0x00000002 //!< Has this process ran at least once?
#define TYPE_VALID    0x00000004 //!< Is this a valid process? Can it be overwritten?
#define TYPE_ZOMBIE   0x00000008 //!< Has this task been killed?
#define TYPE_REAP     0x00000010 //!< Should this task be reaped?
#define TYPE_KERNEL   0x80000000 //!< Does this process run in kernel land?


struct kproc //!< Structure of a process as seen by the kernel
{
	char name[64]; //!< Name of the process
	int pid;       //!< Process ID
	int uid;       //!< User who `owns` the process
	int gid;       //!< Group who `owns` the process

	u32 type;      //!< Type of process

	int children[MAX_CHILDREN]; //!< Indicies of direct child processes (ex: NOT children's children)

#if  defined(ARCH_X86)
	u32 esp;       //!< Stack pointer
	u32 ebp;       //!< Stack base pointer
	u32 eip;       //!< Instruction pointer
	u32 cr3;       //!< Page directory

	u32 stack_beg; //!< Beginning of stack
	u32 stack_end; //!< Current end of stack
#endif

	int exitcode;  //!< Exit code
};

#endif