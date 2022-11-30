#ifndef PROC_TYPES_EXEC_H
#define PROC_TYPES_EXEC_H

#include <stdint.h>

#include <mm/mmap.h>
#include <mm/mmu.h>
#include <proc/types/elf.h>
#include <proc/types/kproc.h>

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

#endif

