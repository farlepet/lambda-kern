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
 * Wait for child process to exit.
 * 
 * @param stat_loc Pointer to variable in which to store child exit information, or NULL.
 * @returns PID of child process
 */
int wait(int *stat_loc);

#endif

