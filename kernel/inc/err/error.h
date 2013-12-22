#ifndef ERROR_H
#define ERROR_H

typedef enum  //!< Enumeration of error levels
{
	ERR_DETAIL,   //!< Just a message containing kernel details
	ERR_INFO,     //!< Message containing kernel info
	ERR_BOOTINFO, //!< Message containing information about the kernel's boot process
	ERR_SMERR,    //!< A small, easily ignored error
	ERR_MEDERR,   //!< A slightly worse error, but can USUALLY be ignored
	ERR_LGERR,    //!< Great error, system can continue, but a process may have to be killed
	ERR_HALTING   //!< Kernel cannot continue operation bacause of this error
} error_level;

/**
 * \brief Prints information about the kernel.
 * Disables interrupts so it will not get interrupted, checks to see if the 
 * error level is >= the minimum level, and if so, prints the current
 * clock tick, then prints the error message. Then, if en_int != 0, it
 * enables interrupts again.
 * @param errlvl the severity of the message
 * @param en_int wether or not to enable interrupts when its done
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kerror(error_level errlvl, int en_int, char *msg, ...);

#endif