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

// TODO: Document
void exec_replace_process_image(void *entryp, const char *name, void *pagedir, symbol_t *symbols, char *symbol_string_table, const char **argv, const char **envp);

#endif