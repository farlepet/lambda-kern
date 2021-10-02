#ifndef KERN_CMDLINE_H
#define KERN_CMDLINE_H

/**
 * @brief Sets the pointer to the kernel commandline
 * 
 * @param cmdline Pointer to commandline string
 */
void cmdline_set(const char *cmdline);

/**
 * @brief Initialize and parse kernel commandine. Requires that memory-management
 * be setup first.
 */
void cmdline_init(void);

/**
 * @brief Handle architecture-independant kernel options
 */
void cmdline_handle_common(void);

/**
 * @brief Get a string value from commandline, in format of <var>=<value>
 * 
 * @note At present, this function returns a pointer into the commandline array,
 * so illegal modifications will affect future calls.
 * 
 * @param var Variable to search for
 * 
 * @return Value of variable if found, else NULL 
 */
const char *cmdline_getstr(const char *var);

/**
 * @brief Gets boolean value from commandline variable
 * 
 * TRUE:
 *   <var>
 *   <var>=true
 *   <var>=TRUE
 * FALSE:
 *   otherwise
 * 
 * @param var Name of variable to find
 * @return 1 if true, 0 if false
 */
int cmdline_getbool(const char *var);

#endif
