#ifndef PROC_EXEC_H
#define PROC_EXEC_H

/**
 * Replace current process with a new process crated using the provided
 * executable, arguments, and environment
 * 
 * @param filename Filename of executable
 * @param argv NULL-terminated list of string arguments (argv[0] same as filename)
 * @param envp NULL-terminated list of environment variables
 * 
 * @return No return on success, -1 on error
 */
int execve(const char *filename, const char **argv, const char **envp);

/**
 * @brief Replace current process with given 
 * 
 * TODO: Remove x86-specific page directory reference
 * 
 * @param entryp New entrypoint of process, where to resume execution in the new image
 * @param name What to rename the process
 * @param pagedir Page directory of new image
 * @param symbols Symbols present in new process image
 * @param symbol_string_table Symbol string table for new image
 * @param argv Argument array to pass new image
 * @param envp Environment array to pass new image
 */
void exec_replace_process_image(void *entryp, const char *name, void *pagedir, symbol_t *symbols, char *symbol_string_table, const char **argv, const char **envp);

/**
 * Wait for child process to exit.
 * 
 * @param stat_loc Pointer to variable in which to store child exit information, or NULL.
 * @returns PID of child process
 */
int wait(int *stat_loc);

#endif