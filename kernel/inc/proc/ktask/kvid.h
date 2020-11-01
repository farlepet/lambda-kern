#ifndef KTASK_KVID_H
#define KTASK_KVID_H

#include <types.h>

enum kvid_types
{
	KVID_PRINT  = 0, //!< Print a message to the screen
	KVID_KERROR = 1  //!< Print information to the screen
};

struct kvid_type_msg //!< A type message to be sent to kvid
{
	int pid;  //!< Sender's PID
	uint8_t  type; //!< Type of kvid message
};

struct kvid_print_msg //!< A message telling kvid to print a string
{
	char *string; //!< A null-terminated string
};

struct kvid_kerror_msg //!< A message telling kvid to print a kernel message
{
	uint32_t error_level; //!< Error level for kerror
	char *string;    //!< String for kerror to print
};

struct kvid_print_m //!< A structure holding an entire kvid print message
{
	struct kvid_type_msg  ktm;
	struct kvid_print_msg kpm;
};

struct kvid_kerror_m //!< S structure holding an entire kvid kerror message
{
	struct kvid_type_msg   ktm;
	struct kvid_kerror_msg kkm;
};

__noreturn void kvid_task(void);

#endif // KTASK_KVID_H
