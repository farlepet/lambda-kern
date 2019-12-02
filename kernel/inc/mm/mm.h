#ifndef MM_H
#define MM_H

#include <types.h>
#include <multiboot.h>
#include <proc/proc.h>

#if defined(ARCH_X86)
#include <mm/paging.h>
#endif

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_head the multiboot header location
 */
void mm_init(struct multiboot_header *mboot_head);

/**
 * \brief Check if memory address is valid.
 * 
 * @param addr Address to check
 * 
 * @return 1 if readable, 2 if writable, else 0
 */
int mm_check_addr(void *addr);

/**
 * @brief Translate process-given address to physical address
 * 
 * @param proc Process from which address originated
 * @param addr Address to translate
 * 
 * @return void* Physical address
 */
void *mm_translate_proc_addr(struct kproc *proc, const void *addr);

#endif