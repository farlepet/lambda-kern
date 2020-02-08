/** \file paging.h
 *  \brief Contains functions dealing with paging.
 *
 *  Contains functions that allow you to set up paging, allocate pages, and
 *  manage page directories.
 */

#ifndef PAGING_H
#define PAGING_H

#include <types.h>


extern uint32_t kern_start; //!< Start address of the kernel
extern uint32_t kern_end;   //!< End address of the kernel

uint32_t kernel_cr3;        //!< Page directory used by the kernel

uint32_t *firstframe;       //!< The location of the first page frame

/**
 * @brief Block page from use
 * 
 * @param page Page frame to block
 */
void block_page(uint32_t page);


/**
 * Sets the current frame to be used or unused, depending on `val`.
 * 
 * @param frame the frame to be set
 * @param val wether the frame is used or unused
 */
void set_frame(uint32_t frame, uint32_t val);

/**
 * \brief Allocate a frame.
 * 
 * Allocates a page frame then returns its address
 */
void *alloc_frame(void);

/**
 * \brief Free an allocated frame.
 * 
 * Free an allocated page frame, allowing it to be allocated again.
 */
void free_frame(void *frame);

/**
 * \brief Map a virtual address to a physical one.
 * 
 * Map a virtual address to a physical one.
 * 
 * @param physaddr physical address to be mapped to
 * @param virtualaddr virtual address to map
 * @param flags information about the page mapping
 * 
 * @see pgdir_map_page
 */
void map_page(void *physaddr, void *virtualaddr, uint32_t flags);

/**
 * @brief Map a page to a physical address, for a given page directory
 * 
 * @param pgdir Page directory
 * @param physaddr Physical address
 * @param virtualaddr Virtual address
 * @param flags Flags to give mapping
 */
void pgdir_map_page(uint32_t *pgdir, void *physaddr, void *virtualaddr, uint32_t flags);

/**
 * @brief Get the physical address from virtual one for current page directory
 * 
 * @param virtaddr Virtual address
 * @return void* Physical address
 */
void *get_phys_page(void *virtaddr);

/**
 * @brief Get page table entry for a given virtual address for the current page directory
 * 
 * @param virtaddr Virtual address
 * @return uint32_t Page table entry
 */
uint32_t get_page_entry(const void *virtaddr);

/**
 * @brief Get page table entry for a given virtual address for a given page directory
 * 
 * @param pgdir Page directory
 * @param virtaddr Virtual address
 * @return uint32_t Page table entry
 */
uint32_t pgdir_get_page_entry(uint32_t *pgdir, const void *virtaddr);

/**
 * @brief Get page directory entry (page table) for given virtual address
 * 
 * @param pgdir Page directory
 * @param virtaddr Virtual address
 * @return uint32_t Page directory entry
 */
uint32_t pgdir_get_page_table(uint32_t *pgdir, const void *virtaddr);

/**
 * @brief Get the physical address from virtual one for a given page directory
 * 
 * @param pgdir Page directory
 * @param virtaddr Virtual address
 * @return uint32_t Physical address
 */
uint32_t pgdir_get_phys_addr(uint32_t *pgdir, const void *virtaddr);

/**
 * \brief Clear a page directory to it's default values.
 * Clear the page directory marking every page table as non-existant.
 */
void clear_pagedir(uint32_t *dir);

/**
 * \brief Fill a page table with pages.
 * Fill a pagetable with pages starting from `addr` and ending up at `addr + 0x400000`
 * @param table pointer to the page table
 * @param addr address to start at
 */
void fill_pagetable(uint32_t *table, uint32_t addr);

/**
 * \brief Set the current page directory
 * Sets the current page directory by setting cr3 to `dir`
 * @param dir the page directory
 */
void set_pagedir(uint32_t *dir);

/**
 * @brief Get current page directory
 * 
 * @return uint32_t* Current page directory
 */
uint32_t *get_pagedir();

/**
 * \brief Checks if the page corresponding to the virtual address exists
 *
 * @return 1 if exists, else 0
 */
int page_present(uint32_t virtaddr);


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
 * Creates a new page directory, and clears it. Thes creates a new page table,
 * and fills it with addresses starting at 0. Then it sets the page tables
 * first page table as the newly created one. Then it sets the page directory,
 * and enables paging.
 * 
 * @param som start of usable memory
 * @param eom end of memory
 * @see clear_pagedir
 * @see fill_pagetable
 * @see enable_paging
 */
void paging_init(uint32_t som, uint32_t eom);

/**
 * @brief Invalidate page
 * 
 * @param addr Address within page
 */
static inline void __invlpg(uint32_t *addr)
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
void *page_alloc(uint32_t size);

/**
 * Free a region of memory that was allocated by page_alloc earlier.
 *
 * @param ptr pointer to memory region
 * @see malloc
 */
void page_free(void *ptr);


/**
 * @brief Shallow clone kernel page directory
 * 
 * @return uint32_t* Clone of page directory
 */
uint32_t *clone_kpagedir();

/**
 * @brief Shallow clone given page directory
 * 
 * @param pgdir Page directory to clone
 * @return uint32_t* Clone of page directory
 */
uint32_t *clone_pagedir(uint32_t * pgdir);

/**
 * @brief Deep clone given page directory
 * 
 * @param pgdir Page directory to clone
 * @return uint32_t* Clone of page directory
 */
uint32_t *clone_pagedir_full(uint32_t *pgdir);

#endif
