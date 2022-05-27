#ifndef PROC_EXEC_H
#define PROC_EXEC_H

#include <proc/types/kproc.h>
#include <proc/elf.h>
#include <mm/mmap.h>
#include <mm/mmu.h>

/**
 * @brief Structure to simplify passing of applicable data in exec methods
 */
typedef struct {
    /** Pointer to buffer containing loaded file */
    void  *file_data;
    /** Size of file buffer */
    size_t file_size;

    /** Program argument list */
    const char **argv;
    /** Program executable list */
    const char **envp;

    /** Process/thread name */
    char      name[KPROC_NAME_MAX];
    /** Thread entrypoint */
    uintptr_t entrypoint;

    /** MMU table for the replacement process */
    mmu_table_t *mmu_table;

    /** Symbols found during program loading */
    symbol_t                 *symbols;
    /** Mapped memory allocated during program loading */
    struct kproc_mem_map_ent *mmap_entries;
    /** ELF-specific data, if applicable */
    proc_elf_data_t          *elf_data;
} exec_data_t;

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