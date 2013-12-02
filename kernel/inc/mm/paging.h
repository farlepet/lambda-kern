#ifndef PAGING_H
#define PAGING_H

#include <types.h>

// Allocate a frame for use
void *alloc_frame();

// Free a frame, making it available for allocation
void free_frame(void *);

// Clear a page directory to default values
void clear_pagedir(u32 *);

// Fill a pagetable with consecutive pages starting at `addr`
void fill_pagetable(u32 *, u32);

// Set the current page directory
void set_pagedir(u32 *);

// Enable paging
void enable_paging();

// Disable paging
void disable_paging();

// Initialize paging
void paging_init();

#endif