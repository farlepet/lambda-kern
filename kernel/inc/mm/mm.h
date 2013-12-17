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

/**
 * \brief Allocate a page.
 * Allocate a page the size of the target architecture's pages and using the
 * target architecture's page allocator. Can later be freed by `free_page`
 * @return the location of the page
 * @see free_frame
 */
void *alloc_page();

/**
 * \brief Free an allocated page.
 * Free a page allocated by `alloc_page`
 * @param page pointer to the page to free
 * @see alloc_page
 */
void free_page(void *page);


#endif