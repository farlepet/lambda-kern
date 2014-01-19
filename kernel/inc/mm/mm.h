#ifndef MM_H
#define MM_H

#include <types.h>
#include <multiboot.h>

#if defined(ARCH_X86)
#include <mm/paging.h>
#endif

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_head the multiboot header location
 */
void mm_init(struct multiboot_header *mboot_head);


#endif