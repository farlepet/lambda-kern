/** \file paging.h
 *  \brief Contains functions dealing with paging.
 *
 *  Contains functions that allow you to set up paging, allocate pages, and
 *  manage page directories.
 */

#ifndef PAGING_H
#define PAGING_H

#include <types.h>


extern u32 kern_start; //!< Start address of the kernel
extern u32 kern_end;   //!< End address of the kernel

u32 kernel_cr3;        //!< Page directory used by the kernel

u32 *firstframe;       //!< The location of the first page frame

void block_page(u32 page);


/**
 * Sets the current frame to be used or unused, depending on `val`.
 * 
 * @param frame the frame to be set
 * @param val wether the frame is used or unused
 */
void set_frame(u32 frame, u32 val);

/**
 * \brief Allocate a frame.
 * Allocates a page frame then returns its address
 */
void *alloc_frame(void);

/**
 * \brief Free an allocated frame.
 * Free an allocated page frame, allowing it to be allocated again.
 */
void free_frame(void *frame);

/**
 * \brief Map a virtual address to a physical one.
 * Map a virtual address to a physical one.
 * @param physaddr physical address to be mapped to
 * @param virtualaddr virtual address to map
 * @param flags information about the page mapping
 */
void map_page(void *physaddr, void *virtualaddr, u32 flags);

void pgdir_map_page(u32 *pgdir, void *physaddr, void *virtualaddr, u32 flags);

void *get_phys_page(void *virtaddr);

u32 get_page_entry(void *virtaddr);


u32 pgdir_get_page_entry(u32 *pgdir, void *virtaddr);

/**
 * \brief Clear a page directory to it's default values.
 * Clear the page directory marking every page table as non-existant.
 */
void clear_pagedir(u32 *dir);

/**
 * \brief Fill a page table with pages.
 * Fill a pagetable with pages starting from `addr` and ending up at `addr + 0x400000`
 * @param table pointer to the page table
 * @param addr address to start at
 */
void fill_pagetable(u32 *table, u32 addr);

/**
 * \brief Set the current page directory
 * Sets the current page directory by setting cr3 to `dir`
 * @param dir the page directory
 */
void set_pagedir(u32 *dir);

u32 *get_pagedir();

/**
 * \brief Checks if the page corresponding to the virtual address exists
 *
 * @return 1 if exists, else 0
 */
int page_present(u32 virtaddr);


/**
 * \brief Enable paging.
 * Enable the paging flag in cr0.
 */
void enable_paging(void);

/**
 * \brief Disable paging.
 * Sets the paging flag in cr0.
 */
void disable_paging(void);

/**
 * \brief Initialize paging.
 * Creates a new page directory, and clears it. Thes creates a new page table,
 * and fills it with addresses starting at 0. Then it sets the page tables
 * first page table as the newly created one. Then it sets the page directory,
 * and enables paging.
 * @param eom end of memory
 * @see clear_pagedir
 * @see fill_pagetable
 * @see enable_paging
 */
void paging_init(u32 eom);


static inline void __invlpg(u32 *addr)
{
	asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}


/**
 * Allocate a region of memory to be used later. VERY basic, only to be used
 *   to set up things for `kmalloc` later on
 *
 * @param size size of the memory region
 * @return address of the memory region
 */
void *page_alloc(u32 size);

/**
 * Free a region of memory that was allocated by page_alloc earlier.
 *
 * @param ptr pointer to memory region
 * @see malloc
 */
void page_free(void *ptr);




u32 *clone_kpagedir();

#endif
