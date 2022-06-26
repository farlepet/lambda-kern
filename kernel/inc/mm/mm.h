#ifndef MM_H
#define MM_H

#include <types.h>
#include <stdint.h>
#include <proc/proc.h>

/**
 * @brief Sets up page mapping on kernel sections
 *
 * @return int 0 on success, else non-zero
 */
int mm_init_kernel_map(void);

/**
 * \brief Check if memory address is valid.
 * 
 * @param addr Address to check
 * 
 * @return 1 if readable, 2 if writable, else 0
 */
int mm_check_addr(const void *addr);

/**
 * @brief Translate process-given address to physical address
 * 
 * @param proc Process from which address originated
 * @param addr Address to translate
 * 
 * @return void* Physical address
 */
void *mm_translate_proc_addr(struct kproc *proc, const void *addr);

/**
 * \brief Add memory map entry to process.
 * 
 * @param proc Process to add entry to
 * @param phys Physical address
 * @param virt Virtual address
 * @param sz Size
 * 
 * @return 0 on success, non-zero otherwise
 */
int mm_proc_mmap_add(struct kproc *proc, uintptr_t phys, uintptr_t virt, size_t sz);

/**
 * \brief Remove memory map entry from process by virtual address.
 * 
 * @param proc Process to remove entry from
 * @param virt Virtual address of entry
 * 
 * @return 0 on success, non-zero on entry not found
 */
int mm_proc_mmap_remove_virt(struct kproc *proc, uintptr_t virt);

/**
 * \brief Remove memory map entry from process by physical address.
 * 
 * @param proc Process to remove entry from
 * @param phys Physical address of entry
 * 
 * @return 0 on success, non-zero on entry not found
 */
int mm_proc_mmap_remove_phys(struct kproc *proc, uintptr_t phys);

#endif