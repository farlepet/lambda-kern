#ifndef PROC_H
#define PROC_H

#include "mtask.h"

#define MAX_PROCESSES 32 //!< Maximum amount of running processes

#define TYPE_RUNNABLE 0x00000001 //!< Is this process runnable?
#define TYPE_RANONCE  0x00000002 //!< Has this process ran at least once?
#define TYPE_VALID    0x00000004 //!< Is this a valid process? Can it be overwritten?
#define TYPE_KERNEL   0x80000000 //!< Does this process run in kernel land?


struct kproc //!< Structure of a process as seen by the kernel
{
	char name[64]; //!< Name of the process
	int pid;       //!< Process ID
	int uid;       //!< User who `owns` the process
	int gid;       //!< Group who `owns` the process

	u32 type;      //!< Type of process

#if  defined(ARCH_X86)
	u32 esp;       //!< Stack pointer
	u32 ebp;       //!< Stack base
	u32 eip;       //!< Instruction pointer
	u32 cr3;       //!< Page directory
#endif
};

#endif