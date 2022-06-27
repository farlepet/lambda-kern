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

extern uint32_t kernel_cr3;        //!< Page directory used by the kernel

#define PAGE_DIRECTORY_FLAG_PRESENT  (1UL << 0)
#define PAGE_DIRECTORY_FLAG_WRITABLE (1UL << 1)
#define PAGE_DIRECTORY_FLAG_USER     (1UL << 2)
#define PAGE_DIRECTORY_FLAG_WRCACHE  (1UL << 3)
#define PAGE_DIRECTORY_FLAG_NOCACHE  (1UL << 4)
#define PAGE_DIRECTORY_FLAG_ACCESSED (1UL << 5)
#define PAGE_DIRECTORY_FLAG_4MBPAGE  (1UL << 7)

#define PAGE_TABLE_FLAG_PRESENT  (1UL << 0)
#define PAGE_TABLE_FLAG_WRITABLE (1UL << 1)
#define PAGE_TABLE_FLAG_USER     (1UL << 2)
#define PAGE_TABLE_FLAG_WRCACHE  (1UL << 3)
#define PAGE_TABLE_FLAG_NOCACHE  (1UL << 4)
#define PAGE_TABLE_FLAG_ACCESSED (1UL << 5)
#define PAGE_TABLE_FLAG_DIRTY    (1UL << 6)
#define PAGE_TABLE_FLAG_GLOBAL   (1UL << 8)


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
uint32_t pgdir_get_page_entry(const uint32_t *pgdir, const void *virtaddr);

/**
 * @brief Get page directory entry (page table) for given virtual address
 * 
 * @param pgdir Page directory
 * @param virtaddr Virtual address
 * @return uint32_t Page directory entry
 */
uint32_t pgdir_get_page_table(uint32_t *pgdir, const void *virtaddr);

/**
 * \brief Fill a page table with identity-mapped pages.
 * Fill a pagetable with pages starting from `addr` and ending up at `addr + 0x400000`
 * @param table pointer to the page table
 * @param addr address to start at
 * @param flags flags to give each page table entry
 */
void fill_pagetable(uint32_t *table, uint32_t addr, uint32_t flags);

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
uint32_t *get_pagedir(void);

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

#endif
