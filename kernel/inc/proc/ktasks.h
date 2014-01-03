#ifndef KTASKS_H
#define KTASKS_H

#include <proc/mtask.h>
#include <types.h>

#define IDLE_TASK_SLOT   0
#define KVID_TASK_SLOT   1
#define KBUG_TASK_SLOT   2
#define KINPUT_TASK_SLOT 3

#define KTASK_SLOTS    4

int ktask_pids[KTASK_SLOTS];


enum kvid_types
{
	KVID_PRINT, //!< Print a message to the screen
	KVID_KERROR //!< Print information to the screen
};

struct kvid_type_msg //!< A type message to be sent to kvid
{
	int pid;  //!< Sender's PID
	u8  type; //!< Type of kvid message
};

struct kvid_print_msg //!< A message telling kvid to print a string
{
	char *string; //!< A null-terminated string
};

struct kvid_kerror_msg //!< A message telling kvid to print a kernel message
{
	u32 error_level; //!< Error level for kerror
	char *string;    //!< String for kerror to print
};



#ifdef DEBUGGER

enum kbug_types
{
	KBUG_PROCINFO, //!< Request info about a process
	KBUG_CPUINFO   //!< Request info about the CPU
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

#endif // Debugger

void init_ktasks();


#endif