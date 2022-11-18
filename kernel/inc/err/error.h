#ifndef ERROR_H
#define ERROR_H

/** Enumeration of error levels */
typedef enum {
    ERR_ALL = 0,   /*!< All - Extrenely high detail */
    ERR_TRACE,     /*!< Trace - Highly detailed debugging, could significantly slow down the kernel */
    ERR_DEBUG,     /*!< Debug - More detailed debug info, and warnings in syscalls */
    ERR_INFO,      /*!< Information - Standard debug level, generally useful information messages */
    ERR_NOTICE,    /*!< Notice - Might be an issue, but not to the level of warning, */
    ERR_WARN,      /*!< Warning - Potential issue that doesn't yet effect the stability of the system */
    ERR_ERROR,     /*!< Error - Potentially impacts the stability of the system */
    ERR_CRIT,      /*!< Critical error - Definitely heavily impacts the stability of a portion of the system. */
    ERR_EMER,      /*!< Emergency - The system cannot continue */

    ERR_NONE = 255 /*!< None - Do not log any debug information */
} error_level_e;


/** Enumeration of debug message sources */
typedef enum {
    DEBUGSRC_MISC    = 0, /*!< Miscellaneous/uncategorized */
    DEBUGSRC_FS      = 1, /*!< Kernel filesystem interface */
    DEBUGSRC_MM      = 2, /*!< Memory management */
    DEBUGSRC_PROC    = 3, /*!< Process management */
    DEBUGSRC_EXEC    = 4, /*!< Process creation and execution */
    DEBUGSRC_SYSCALL = 5, /*!< System calls */
    DEBUGSRC_MODULE  = 6, /*!< Module loading/unloading and management */

    DEBUGSRC_MAX
} debug_source_e;

/**
 * \brief Prints debug info from a specified kernel source
 * 
 * Similar to kerror, except it checks if the corresponding bit is set in the
 * debug mask rather than checking error level.
 * 
 * @note It may be better to implement this as a pre-compiler check, so this
 * function isn't called in time-sensitive or frequent operations if debugging
 * is occasionally desired there.
 * 
 * @param src the source of the message
 * @param lvl debug level
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kdebug(debug_source_e src, error_level_e lvl, const char *msg, ...);

#define kerror(...) kdebug(DEBUGSRC_MISC, __VA_ARGS__)

/**
 * @brief Set debug level filter for a given debug source
 * 
 * @param src Debug source
 * @param lvl Debug level
 */
void kdebug_set_errlvl(debug_source_e src, error_level_e lvl);

/**
 * @brief Get current debug level filter for a given debug source
 * 
 * @param src Debug source
 */
error_level_e kdebug_get_errlvl(debug_source_e src);

#endif