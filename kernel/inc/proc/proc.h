#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <proc/types/kproc.h>
#include <proc/types/kthread.h>

#include <data/llist.h>
#include <fs/kfile.h>
#include <mm/symbols.h>
#include <mm/mmu.h>
#include <proc/syscalls.h>
#include <proc/elf.h>



/**
 * \brief Adds file to process
 * 
 * @param proc Process to add file to
 * @param hand File handle to add
 * 
 * @return New file descriptor if applicable, -1 otherwise
 */
int proc_add_file(struct kproc *proc, kfile_hand_t *hand);

/**
 * \brief Add child index to parent
 * 
 * @param parent Pointer to parent process struct
 * @param child Pointer to child to add
 * 
 * @return 0 on success, else 1
 */
int proc_add_child(struct kproc *parent, struct kproc *child);

/**
 * \brief Allocate and initialize a process structure 
 * 
 * @param name Name of the process
 * @param domain Which domain this process runs in
 * @param mmu_table MMU table to use, NULL to create new one
 *
 * @return Null on error, else pointer to newly created process
 */
kproc_t *proc_create(char *name, int domain, mmu_table_t *mmu_table);

/**
 * \brief Initialize scheduler
 * 
 * @param n_cpus Number of available CPUs
 * @return 0 on success, else non-zero
 */
int sched_init(unsigned n_cpus);

/**
 * \brief Create idle thread for each CPU
 */
void sched_idle_init(void);

/**
 * \brief Reschedule processes
 *
 * Assigns processes currently awaiting scheduling to a CPU
 */
void sched_processes(void);

/**
 * @brief Select next process to execute
 */
kthread_t *sched_next_process(unsigned cpu);

/**
 * \brief Add thread to scheduling queue
 * 
 * @param thread Thread to add to queue
 * @return 0 on success, else failure
 */
int sched_enqueue_thread(kthread_t *thread);

/**
 * @brief Remove thread from scheduler
 *
 * @param thread Thread to remove
 *
 * @return int 0 on success, else non-zero
 */
int sched_remove_thread(kthread_t *thread);

/**
 * \brief Get currently active thread on the requested CPU
 * 
 * @param cpu CPU to get actuve thread of
 * @return NULL on error, else pointer to active thread on CPU
 */
kthread_t *sched_get_curr_thread(unsigned cpu);

/**
 * @brief Replaces current thread with another, does not free thread
 *
 * @note This must only be called when interrupts are disabled
 *
 * @see execve
 *
 * @param new_thread New thread to take the place of the current one
 */
void sched_replace_thread(kthread_t *new_thread);

/**
 * \brief Add memory map record to process
 * 
 * @param proc Process to add record to
 * @param virt_address Virtual address memory is mapped to
 * @param phys_address Physical address
 * @param length Length of contiguous region, in bytes
 * @return int 0 if successful
 */
int proc_add_mmap_ent(struct kproc *proc, uintptr_t virt_address, uintptr_t phys_address, size_t length);

/**
 * \brief Add multiple memory map records to a process
 * 
 * @param entries Linked-list of memory map entries to add.
 * @return int 0 on success
 */
int proc_add_mmap_ents(struct kproc *proc, kproc_mem_map_ent_t *entries);

/**
 * \brief Add thread to a process
 * 
 * @param proc Process to add thread to
 * @param thread New thread to add
 * 
 * @return 0 on success, else non-zero
 */
int proc_add_thread(kproc_t *proc, kthread_t *thread);

/**
 * @brief Destroy process and free memory
 *
 * @param proc Process to destroy
 *
 * @return int 0 on success, else non-zero
 */
int proc_destroy(kproc_t *proc);

#endif
