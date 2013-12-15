/** \file paging.h
 *  \brief Contains functions dealing with paging.
 *
 *  Contains functions that allow you to set up paging, allocate pages, and
 *  manage page directories.
 */

#ifndef PAGING_H
#define PAGING_H

#include <types.h>


extern u32 kern_start;          //!< Start address of the kernel
extern u32 kern_end;            //!< End address of the kernel

/**
 * \brief Allocate a frame.
 * Allocates a page frame then returns its address
 */
void *alloc_frame();

/**
 * \brief Free an allocated frame.
 * Free an allocated page frame, allowing it to be allocated again.
 */
void free_frame(void *frane);

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

/**
 * \brief Enable paging.
 * Enable the paging flag in cr0.
 */
void enable_paging();

/**
 * \brief Disable paging.
 * Sets the paging flag in cr0.
 */
void disable_paging();

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

#endif