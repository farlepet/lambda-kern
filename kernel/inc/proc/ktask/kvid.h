#ifndef KTASK_KVID_H
#define KTASK_KVID_H

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

void kvid_task(void);

#endif // KTASK_KVID_H