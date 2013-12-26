#include <types.h>
#include <video.h>
#include "paging.h"
#include "mem.h"

static u32 *pagedir;            //!< Main kernel pagedirectory
static u32 *pagetbl1;           //!< First page table
static u32 *frames;             //!< Table stating which frames are available
static u32 prealloc_frames[20]; //!< 20 frames that are free, used by alloc_frame
static u32 nframes;             //!< Number of frames in the table

static u32 *firstframe;         //!< The location of the first page frame

/**
 * \brief Makes the frame unavailable or available.
 * Sets the current frame to be used or unused, depending on `val`.
 * @param frame the frame to be set
 * @param val wether the frame is used or unused
 */
static void set_frame(u32 frame, u32 val)
{
	if(val) frames[frame / 32] |=  (1 >> (frame % 32));
	else    frames[frame / 32] &= ~(1 >> (frame % 32));
}

/**
 * \brief Checks if the frame is available
 * Checks if the frame is not being used.
 * @param frame the frame to be tested
 */
static u32 test_frame(u32 frame)
{
	return ((frames[frame / 32] & (1 >> (frame % 32))) != 0);
}

/**
 * \brief Get the first free frame.
 * Look through ass the pages, and see if any are available.
 */
static void *get_free_frame()
{
		u32 i = 0;
		while(test_frame(i) != 0)
		{
			if(frames[i/32] == 0xFFFFFFFF)
			{
				i += 32;
				continue;
			}
			if(i == nframes)
				return (void *)0xFFFFFFFF; // Error, no pages available
			i++;
		}

		set_frame(i, 1);
		return (firstframe + (i*0x1000));
}

/**
 * \brief Allocate a frame.
 * Allocates a page frame then returns its address
 */
void *alloc_frame()
{
		static u32 pframe = 20;
 
		if(pframe == 20)
		{
			int i = 0;
			for(; i < 20; i++)
			{
				prealloc_frames[i] = (u32)get_free_frame();
				if(prealloc_frames[i] == 0xFFFFFFFF) return (void *)0xFFFFFFFF; // TODO: Add some type of handling system here
			}
			pframe = 0;
		}
		return (void *)prealloc_frames[pframe++];
}

/**
 * \brief Map a virtual address to a physical one.
 * Map a virtual address to a physical one.
 * @param physaddr physical address to be mapped to
 * @param virtualaddr virtual address to map
 * @param flags information about the page mapping
 */
void map_page(void *physaddr, void *virtualaddr, u32 flags)
{
	// Make sure that both addresses are page-aligned.
	
	u32 pdindex = (u32)virtualaddr >> 22;
	u32 ptindex = (u32)virtualaddr >> 12 & 0x03FF;
	
	// Should I check if it is already present? I don't know...
	if(pagedir[pdindex] & 0x01)
		((u32 *)pagedir[pdindex])[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	else
	{
		pagedir[pdindex] = (u32)alloc_frame() | 0x03;
		((u32 *)pagedir[pdindex])[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	}
}

/**
 * \brief Free an allocated frame.
 * Free an allocated page frame, allowing it to be allocated again.
 */
void free_frame(void *frame)
{
	u32 f = (u32)frame;
	f -= (u32)firstframe;
	f /= 0x1000;
	set_frame(f, 0);
}



/**
 * \brief Clear a page directory to it's default values.
 * Clear the page directory marking every page table as non-existant.
 */
void clear_pagedir(u32 *dir)
{
	int i = 0;
	for(i = 0; i < 1024; i++)
		dir[i] = 2; // supervisor, rw, not present.
}

/**
 * \brief Fill a page table with pages.
 * Fill a pagetable with pages starting from `addr` and ending up at `addr + 0x400000`
 * @param table pointer to the page table
 * @param addr address to start at
 */
void fill_pagetable(u32 *table, u32 addr)
{
	u32 i;
	for(i = 0; i < 1024; i++, addr += 0x1000)
		table[i] = addr | 3;  // supervisor, rw, present.
}

/**
 * \brief Set the current page directory
 * Sets the current page directory by setting cr3 to `dir`
 * @param dir the page directory
 */
void set_pagedir(u32 *dir)
{
	asm volatile("mov %0, %%cr3":: "b"(dir));
}

/**
 * \brief Enable paging.
 * Enable the paging flag in cr0.
 */
void enable_paging()
{
	u32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

/**
 * \brief Disable paging.
 * Sets the paging flag in cr0.
 */
void disable_paging()
{
	u32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 &= ~0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

extern u32 apic;
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
void paging_init(u32 eom)
{
	pagedir    = (u32 *)FIRST_PAGEDIR;
	pagetbl1   = (u32 *)FIRST_PAGETBL;
	
	frames     = (u32 *)FRAMES_TABLE;
	firstframe = (u32 *)FRAMES_START;

	nframes = (u32)(eom - (u32)firstframe) / 0x1000;
	
	u32 i = 0;
	for(; i < nframes; i++, frames[i] = 0);

	clear_pagedir(pagedir);
	fill_pagetable(pagetbl1, 0x00000000);
	pagedir[0] = (u32)pagetbl1 | 3; // supervisor, rw, present

	set_pagedir(pagedir);
	enable_paging();
}