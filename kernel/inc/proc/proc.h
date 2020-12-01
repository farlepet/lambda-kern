#ifndef PROC_H
#define PROC_H

struct kproc;
struct uproc;

#define MAX_PROCESSES        16 //!< Maximum amount of running processes
#define MAX_CHILDREN         8  //!< Maximum number of children a parent can handle
#define MAX_PROCESS_MESSAGES 64 //!< Maximum number of messages a process can retain
#define MAX_BLOCKED_PIDS     (MAX_PROCESSES - 1)
#define MAX_OPEN_FILES       8  //!< Maximum number of files opened by any particular process, including 0-2

#define MSG_BUFF_SIZE 512 //!< Size of the message buffer in bytes

#define PROC_KERN_STACK_SIZE 4096 //!< Size of kernel stack allocated to process

#define TYPE_RUNNABLE 0x00000001 //!< Is this process runnable?
#define TYPE_RANONCE  0x00000002 //!< Can this process save its registers yet?
#define TYPE_VALID    0x00000004 //!< Is this a valid process? Can it be overwritten?
#define TYPE_ZOMBIE   0x00000008 //!< Has this task been killed?
#define TYPE_REAP     0x00000010 //!< Should this task be reaped?
#define TYPE_KERNEL   0x80000000 //!< Does this process run in kernel land?

#define BLOCK_DELAY       0x00000001 //!< Process is blocked waiting for a delay
#define BLOCK_MESSAGE     0x00000002 //!< Process is blocked waiting for a message
#define BLOCK_IPC_MESSAGE 0x00000004 //!< Process is blocked waiting for a message (mew IPC style)
#define BLOCK_WAIT        0x00000008 //!< Process is blocked waiting for a child process to exit

#define PRIO_IDLE       0 //!< Only idle processes use this priority
#define PRIO_USERPROG   1 //!< Priority for user programs
#define PRIO_KERNELPROG 2 //!< Priority for kernel programs
#define PRIO_DRIVER     3 //!< Priority for kernel drivers
#define PRIO_KERNEL     4 //!< Priority for main kernel tasks

#include <stdint.h>

#include <mm/cbuff.h>
#include <fs/kfile.h>
#include <mm/symbols.h>
#include <proc/syscalls.h>
#include <proc/elf.h>

#include <arch/proc/tasking.h>

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

typedef struct proc_elf_data {
	/* Dynamic linker data: */
	const Elf32_Dyn *dynamic;
	const char      *dynamic_str;     //!< Dynamic string table
	size_t           dynamic_str_len; //!< Dynamic string table length
	const Elf32_Sym *dynamic_sym;     //!< Dynamic symbol table
} proc_elf_data_t;

/* TODO: Convert some static-size arrays in kproc to dynamically allocated memory. */
struct kproc { //!< Structure of a process as seen by the kernel
	char          name[64]; //!< Name of the process
	int           pid;      //!< Process ID
	int           uid;      //!< User who `owns` the process
	int           gid;      //!< Group who `owns` the process

	/* TODO: Currently parent anc childred are completely different, perhaps
	 * they should both be pointers? */
	int           parent;   //!< PID of parent process

	uint32_t      type;     //!< Type of process

	struct kproc *children[MAX_CHILDREN]; //!< Pointers to direct child processes (ex: NOT children's children)

	kproc_arch_t  arch; //!< Architecture-specific process data

	uint32_t      entrypoint; //!< Program start

	/* Old IPC method. TODO: remove. */
	struct cbuff  messages;           //!< Message buffer structure
	uint8_t       msg_buff[MSG_BUFF_SIZE]; //!< Actual buffer

	struct kfile *cwd; //!< Current working directory

	struct kfile *open_files[MAX_OPEN_FILES]; //!< Open file descriptors
	uint32_t      file_position[MAX_OPEN_FILES]; //!< Current position in open files

	symbol_t     *symbols;   //!< Symbol names used to display a stack trace
	char         *symStrTab; //!< Strings used for symbol table

	proc_elf_data_t *elf_data; //!< Data specific for ELF executables

	/* TODO: Might be best to simply deprecate this, similar could be done
	 * using pipes, or other standard IPC techniques. Perhaps implement more
	 * standard POSIX message queues. */
	struct ipc_message *ipc_messages[MAX_PROCESS_MESSAGES]; //!< IPC message pointers
	int                 blocked_ipc_pids[MAX_BLOCKED_PIDS]; //!< PIDs blocked from sending messages to this process

	uint32_t      blocked;   //!< Contains flags telling whether or not this process is blocked, and by what

	int           exitcode;  //!< Exit code

	int           prio;      //!< Task priority

	struct        proc_book book; //!< Bookkeeping stuff

	struct kproc_mem_map_ent *mmap; //!< Memory map


	struct kproc *next; /*!< Next process in linked list. */
	struct kproc *prev; /*!< Previous process in linked list. */
};


struct uproc { //!< Structure of a process as seen by a user process
	char name[64]; //!< Name of the process
	int pid;       //!< Process ID
	int uid;       //!< User who `owns` the process
	int gid;       //!< Group who `owns` the process

	uint32_t type;      //!< Type of process

	int children[MAX_CHILDREN]; //!< Indexes of direct child processes (ex: NOT children's children)

	uint32_t ip;        //!< Instruction pointer

	uint32_t blocked;   //!< Contains flags telling whether or not this process is blocked, and by what

	int exitcode;  //!< Exit code

	int prio;      //!< Task priority
};

/**
 * @brief Convert kproc structure to uproc structure for transmission to
 * userland application.
 * 
 * @param kp Source kproc structure
 * @param up Destination uproc structure
 */
void kproc_to_uproc(struct kproc *kp, struct uproc *up);

/**
 * \brief Adds file to process
 * 
 * @param proc Process to add file to
 * @param file File to add
 * 
 * @return New file descriptor if applicable, -1 otherwise
 */
int proc_add_file(struct kproc *proc, struct kfile *file);

/**
 * @brief Add child index to parent
 * 
 * @param parent Pointer to parent process struct
 * @param child Pointer to child to add
 * 
 * @return 0 on success, else 1
 */
int proc_add_child(struct kproc *parent, struct kproc *child);

/**
 * @brief Reschedule processes
 * 
 * NOTE: This function currently does nothing!
 */
void sched_processes(void);

/**
 * @brief Select next process to execute
 */
void sched_next_process(void);

/**
 * @brief Add memory map record to process
 * 
 * @param proc Process to add record to
 * @param virt_address Virtual address memory is mapped to
 * @param phys_address Physical address
 * @param length Length of contiguous region, in bytes
 * @return int 0 if successful
 */
int proc_add_mmap_ent(struct kproc *proc, uintptr_t virt_address, uintptr_t phys_address, size_t length);

/**
 * @brief Add multiple memory map records to a process
 * 

// TODO: Documentations to add records to
 * @param entries Linked-list of memory map entries to add.
 * @return int 0 on success
 */
int proc_add_mmap_ents(struct kproc *proc, struct kproc_mem_map_ent *entries);

#endif
