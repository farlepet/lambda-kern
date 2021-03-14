#ifndef ERROR_H
#define ERROR_H

typedef enum { //!< Enumeration of error levels
	ERR_DETAIL,   //!< Just a message containing kernel details
	ERR_INFO,     //!< Message containing kernel info
	ERR_BOOTINFO, //!< Message containing information about the kernel's boot process
	ERR_SMERR,    //!< A small, easily ignored error
	ERR_MEDERR,   //!< A slightly worse error, but can USUALLY be ignored
	ERR_LGERR,    //!< Great error, system can continue, but a process may have to be killed
	ERR_HALTING   //!< Kernel cannot continue operation bacause of this error
} error_level_e;

/**
 * \brief Prints information about the kernel.
 * 
 * Checks to see if the  error level is >= the minimum level, and if so,
 * prints the current clock tick, then prints the error message.
 * 
 * @param errlvl the severity of the message
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kerror(error_level_e errlvl, char *msg, ...);

/** Enumeration of debug message sources */
typedef enum {
	DEBUGSRC_FS      = 0, /** Kernel filesystem interface */
	DEBUGSRC_MM      = 1, /** Memory management */
	DEBUGSRC_PROC    = 2, /** Process management */
	DEBUGSRC_EXEC    = 3, /** Process creation and execution */
	DEBUGSRC_SYSCALL = 4, /** System calls */
	DEBUGSRC_DRIVER  = 5  /** Driver loading/unloading and management */
} debug_source_e;

/**
 * \brief Prints debug info from a specified kernel source
 * 
 * Similar to kerror, except it checks if the corresponding bit is set in the
 * debug mask rather than checking error level.
 * 
 * @param src the source of the message
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 * 
 * @see kerror
 */
void kdebug(debug_source_e src, char *msg, ...);

#endif