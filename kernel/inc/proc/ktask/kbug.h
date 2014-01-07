#ifndef KTASK_KBUG_H
#define KTASK_KBUG_H

#include <types.h>

#ifdef DEBUGGER

enum kbug_types
{
	KBUG_PROCINFO, //!< Request info about a process
	KBUG_CPUINFO,  //!< Request info about the CPU
	KBUG_MEMINFO   //!< Request info about memory
};

enum kbug_proc_type
{
	KBUG_PROC_NPROCS,  //!< Number of currently running processes
	KBUG_PROC_PROCPID, //!< The PID of the selected process
	KBUG_PROC_UPROC    //!< Request a user version of the task structure
};

struct kbug_type_msg //!< Message containing type of next message
{
	int pid; //!< PID of the process who sent this message, so messages can be sent back
	u8 type; //!< Type of request
};

struct kbug_proc_msg //!< Message containing type of process info it requests
{
	int pid;  //!< PID of the process whose information is wanted (not always used)
	u8  type; //!< Type of information it requests
	u32 info; //!< Extra required information (not always used)
};

struct kbug_mem_msg //!< Message containing memory request
{
	max_ptr_t mem_addr; //!< Address of memory to request
	max_ptr_t mem_len;  //!< Length of memory request
};

void kbug_task(void);

#endif // DEBUGGER

#endif // KTASK_KBUG_H