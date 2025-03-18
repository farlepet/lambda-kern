#ifndef PROC_TYPES_KPROC_H
#define PROC_TYPES_KPROC_H

typedef struct kproc kproc_t;
typedef struct proc_elf_data proc_elf_data_t;
typedef struct kproc_mem_map_ent kproc_mem_map_ent_t;

#define KPROC_NAME_MAX 63

#define MAX_PROCESSES        16 //!< Maximum amount of running processes
#define MAX_CHILDREN         8  //!< Maximum number of children a parent can handle
#define MAX_THREADS          16 //!< Maximum number of threads a process can contain
#define MAX_PROCESS_MESSAGES 64 //!< Maximum number of messages a process can retain
#define MAX_BLOCKED_PIDS     (MAX_PROCESSES - 1)
#define MAX_OPEN_FILES       8  //!< Maximum number of files opened by any particular process, including 0-2
#define MSG_BUFF_SIZE 512 //!< Size of the message buffer in bytes

#define TYPE_RUNNABLE (1UL <<  0) //!< Is this process runnable?
#define TYPE_ZOMBIE   (1UL <<  1) //!< Has this task been killed?

#define PROC_DOMAIN_KERNEL    0 /** In-kernel processes */
#define PROC_DOMAIN_MODULE    1 /** Processes originating from external modules */
#define PROC_DOMAIN_USERSPACE 2 /** Processes in userspace */

#include <proc/types/elf.h>
#include <proc/types/kthread.h>

#include <data/types/llist.h>
#include <fs/kfile.h>
#include <mm/mmu.h>
#include <mm/symbols.h>

#include <arch/proc/tasking.h>

struct kproc_mem_map_ent { //!< Memory-map entry
    uintptr_t virt_address; //!< Virtual address of memory location
    uintptr_t phys_address; //!< Physical address of memory location
    size_t    length;       //!< Length of memory location

    struct kproc_mem_map_ent *next; //!< Next memory map entnry in linked-list. NULL if this is the last element
};

struct proc_elf_data {
    /* Dynamic linker data: */
    const Elf32_Dyn *dynamic;
    const char      *dynamic_str;     //!< Dynamic string table
    size_t           dynamic_str_len; //!< Dynamic string table length
    const Elf32_Sym *dynamic_sym;     //!< Dynamic symbol table
};


/* TODO: Convert some static-size arrays in kproc to dynamically allocated memory. */
struct kproc { //!< Structure of a process as seen by the kernel
    char          name[KPROC_NAME_MAX+1]; //!< Name of the process
    int           pid;      //!< Process ID
    int           uid;      //!< User who `owns` the process
    int           gid;      //!< Group who `owns` the process

    /* TODO: Currently parent anc childred are completely different, perhaps
     * they should both be pointers? */
    int           parent;   //!< PID of parent process

    uint32_t      type;     //!< Type of process

    mmu_table_t  *mmu_table; /** MMU table for process */

    uint8_t       domain; /** Process domain, see PROC_DOMAIN_* */

    kproc_arch_t  arch;     /** Architecture-specific process data */

    struct kproc *children[MAX_CHILDREN]; //!< Pointers to direct child processes (ex: NOT children's children)

    llist_t       threads;

    kfile_t      *cwd; //!< Current working directory

    kfile_hand_t *open_files[MAX_OPEN_FILES]; //!< Open file descriptors
    uint32_t      file_position[MAX_OPEN_FILES]; //!< Current position in open files

    symbol_t     *symbols;   //!< Symbol names used to display a stack trace

    proc_elf_data_t *elf_data; //!< Data specific for ELF executables

    int           exitcode;  //!< Exit code

    kproc_mem_map_ent_t *mmap; //!< Memory map

    cond_t *wait_cond; /** Conditional to use when waiting on child to exit */

    llist_item_t list_item;
};

#endif
