#ifndef KTASKS_H
#define KTASKS_H

#include <proc/mtask.h>
#include <types.h>

#define IDLE_TASK_SLOT 0
#define KVID_TASK_SLOT 1

#define KTASK_SLOTS    2

int ktask_pids[KTASK_SLOTS];


enum kvid_types
{
	KVID_PRINT,
	KVID_KERROR
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



void init_ktasks();


#endif