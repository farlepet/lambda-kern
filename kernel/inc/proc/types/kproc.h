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
#define TYPE_KERNEL   (1UL << 31) //!< Is this a kernel process?

#define BLOCK_DELAY       (1UL << 0) //!< Process is blocked waiting for a delay
#define BLOCK_MESSAGE     (1UL << 1) //!< Process is blocked waiting for a message
#define BLOCK_IPC_MESSAGE (1UL << 2) //!< Process is blocked waiting for a message (mew IPC style)
#define BLOCK_WAIT        (1UL << 3) //!< Process is blocked waiting for a child process to exit

#include <proc/types/kthread.h>

#include <data/llist.h>
#include <arch/proc/tasking.h>
#include <proc/elf.h>
#include <fs/kfile.h>

struct proc_book { //!< Structure for process `book-keeping`
	uint32_t sent_msgs;   //!< Number of sent messages
	uint32_t sent_bytes;  //!< Number of sent bytes

	uint32_t recvd_msgs;  //!< Number of received messages
	uint32_t recvd_bytes; //!< Number of received bytes

	uint32_t schedule_count; //!< Number of times this process has been scheduled
	uint32_t syscall_count;  //!< Number of times this process has invoked a syscall
};

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

	kproc_arch_t  arch;     /** Architecture-specific process data */

	struct kproc *children[MAX_CHILDREN]; //!< Pointers to direct child processes (ex: NOT children's children)

	llist_t       threads;

	kfile_t      *cwd; //!< Current working directory

	kfile_hand_t *open_files[MAX_OPEN_FILES]; //!< Open file descriptors
	uint32_t      file_position[MAX_OPEN_FILES]; //!< Current position in open files

	symbol_t     *symbols;   //!< Symbol names used to display a stack trace

	proc_elf_data_t *elf_data; //!< Data specific for ELF executables

	int           exitcode;  //!< Exit code

	struct        proc_book book; //!< Bookkeeping stuff

	kproc_mem_map_ent_t *mmap; //!< Memory map

	llist_item_t list_item;
};

#endif
