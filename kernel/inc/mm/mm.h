#ifndef MM_H
#define MM_H

#include <types.h>
#include <multiboot.h>

/**
 * \brief Initializes memory management.
 * Initializes memory management for the target archetecture.
 * @param mboot_tag the multiboot header location
 */
void mm_init(struct multiboot_header_tag *mboot_tag);

#endif