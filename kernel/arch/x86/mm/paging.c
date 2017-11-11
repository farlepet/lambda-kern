#include <multiboot.h>
#include <err/error.h>
#include <err/panic.h>
#include <intr/int.h>
#include <mm/alloc.h>
#include <string.h>
#include "paging.h"
#include <types.h>
#include <video.h>
#include "mem.h"

static u32 pagedir[1024]      __align(0x1000); //!< Main kernel pagedirectory
static u32 init_tbls[4][1024] __align(0x1000); //!< First 4 page tables
static u32 frames[0x10000]    __align(0x1000); //!< Table stating which frames are available, takes up 256KiB
static u32 prealloc_frames[20];                //!< 20 frames that are free, used by alloc_frame
static u32 nframes;                            //!< Number of frames in the table

u32 kernel_cr3;        //!< Page directory used by the kernel

u32 *firstframe;       //!< The location of the first page frame
static u32 *lastframe; //!< The location of the last page frame

/**
 * Sets the current frame to be used or unused, depending on `val`.
 * 
 * @param frame the frame to be set
 * @param val wether the frame is used or unused
 */
void set_frame(u32 frame, u32 val)
{
	//kerror(ERR_BOOTINFO, "  set_frame(%08X, %08X)", frame, val);
	if(val == 1)
	{
		frames[frame / 32] |=  (1 << (frame % 32));
	}
	else if(val == 0) frames[frame / 32] &= ~(1 << (frame % 32));
	else if(val == 0xFFFFFFFF) // BLOCK this page from use
	{
		frames[frame / 32] |= (1 << (frame % 32));
		u32 addr = (((frame * 0x1000) + (u32)firstframe) & 0xFFFFF000);
		u32 pdindex = addr >> 22;
		u32 ptindex = addr >> 12 & 0x03FF;
		((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] = 0x00000000; // Invalidate the page
	}
	else kerror(ERR_MEDERR, "invalid value to set_frame: %d", val);
}

void block_page(u32 page)
{
	set_frame((page - (u32)firstframe) / 0x1000, 0xFFFFFFFF);
}

/**
 * Checks if the frame is not being used.
 * 
 * @param frame the frame to be tested
 */
static u32 test_frame(u32 frame)
{
	return ((frames[frame / 32] & (1 << (frame % 32))) != 0);
}

/**
 * Look through the pages, and see if any are available.
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

	map_page(firstframe + (i*0x1000), firstframe + (i*0x1000), 3); // Make sure it is mapped
		
	return (firstframe + (i*0x1000));
}

/**
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

void *get_phys_page(void *virtaddr)
{
	void *off = (void *)((u32)virtaddr & 0x00000FFF);
	virtaddr = (void *)((u32)virtaddr & 0xFFFFF000);

	u32 pdindex = (u32)virtaddr >> 22;
	u32 ptindex = (u32)virtaddr >> 12 & 0x03FF;

	if(pagedir[pdindex] & 0x01)
	{
		if(((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] & 0x01)
			return (void *)((((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] & 0xFFFFF000) | (u32)off);
		else
			return NULL;
	}

	else return NULL;
}

int page_present(u32 virtaddr) {
	u32 pdindex = (u32)virtaddr >> 22;
	u32 ptindex = (u32)virtaddr >> 12 & 0x03FF;

    if(pagedir[pdindex] & 0x01) {
		return ((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] & 0x01;
	}

	return 0;
}

u32 get_page_entry(void *virtaddr) {
	u32 pdindex = (u32)virtaddr >> 22;
	u32 ptindex = (u32)virtaddr >> 12 & 0x03FF;

	if(pagedir[pdindex] & 0x01) {
		return ((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex];
	}
	return 0;
}


u32 pgdir_get_page_entry(u32 *pgdir, void *virtaddr) {
	u32 pdindex = (u32)virtaddr >> 22;
	u32 ptindex = (u32)virtaddr >> 12 & 0x03FF;

	if(pgdir[pdindex] & 0x01) {
		return ((u32 *)(pgdir[pdindex] & 0xFFFFF000))[ptindex];
	}
	return 0;
}

/**
 * Map a virtual address to a physical one.
 * 
 * @param physaddr physical address to be mapped to
 * @param virtualaddr virtual address to map
 * @param flags information about the page mapping
 */
void map_page(void *physaddr, void *virtualaddr, u32 flags)
{
	kerror(ERR_BOOTINFO, "Mapping %8X to %8X (%03X)", physaddr, virtualaddr, flags);
	virtualaddr = (void *)((u32)virtualaddr & 0xFFFFF000);
	physaddr    = (void *)((u32)physaddr    & 0xFFFFF000);

	u32 pdindex = (u32)virtualaddr >> 22;
	u32 ptindex = (u32)virtualaddr >> 12 & 0x03FF;

	

	// Should I check if it is already present? I don't know...
	if(pagedir[pdindex] & 0x01)
	{
		kerror(ERR_BOOTINFO, "  -> Page table exists");
		((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	}

	else
	{
		kerror(ERR_BOOTINFO, "  -> Creating new page table");
		pagedir[pdindex] = (((u32)kmalloc(0x2000) + 0x1000) & ~0xFFF) | 0x03;

		int i = 0;
		for(; i < 1024; i++)
			((u32 *)(pagedir[pdindex] & 0xFFFFF000))[i] = 0x00000000;

		((u32 *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	}

	pagedir[pdindex] |= flags & 0x04;
	__invlpg(virtualaddr);
}

void pgdir_map_page(u32 *pgdir, void *physaddr, void *virtualaddr, u32 flags)
{
	kerror(ERR_BOOTINFO, "Mapping %8X to %8X (%03X) in %8X", physaddr, virtualaddr, flags, pgdir);

	virtualaddr = (void *)((u32)virtualaddr & 0xFFFFF000);
	physaddr    = (void *)((u32)physaddr    & 0xFFFFF000);

	u32 pdindex = (u32)virtualaddr >> 22;
	u32 ptindex = (u32)virtualaddr >> 12 & 0x03FF;

	// Should I check if it is already present? I don't know...
	if(pgdir[pdindex] & 0x01)
	{
		kerror(ERR_BOOTINFO, "  -> Page table exists");
		((u32 *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	} else
	{
		kerror(ERR_BOOTINFO, "  -> Creating new page table");
		pgdir[pdindex] = (((u32)kmalloc(0x2000) + 0x1000) & ~0xFFF) | 0x03;

		int i = 0;
		for(; i < 1024; i++)
			((u32 *)(pgdir[pdindex] & 0xFFFFF000))[i] = 0x00000000;

		((u32 *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] = ((u32)physaddr) | (flags & 0xFFF) | 0x01;
	}

	pagedir[pdindex] |= flags & 0x04;
}

/**
 * Free an allocated page frame, allowing it to be allocated again.
 * 
 * @param frame frame to free
 */
void free_frame(void *frame)
{
	set_frame(((u32)frame - (u32)firstframe) / 0x1000, 0);
}



/**
 * Clear the page directory marking every page table as non-existant.
 * 
 * @dir Directory to clear
 */
void clear_pagedir(u32 *dir)
{
	int i = 0;
	for(i = 0; i < 1024; i++)
	{
		kerror(ERR_INFO, "      -> PDIRENT %X", i);
		dir[i] = 2; // supervisor, rw, not present.
	}
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
 * Sets the current page directory by setting cr3 to `dir`
 * 
 * @param dir the page directory
 */
void set_pagedir(u32 *dir)
{
	if(!dir)
	{
		kpanic("Attempted to set pagedir pointer to NULL!");
	}

	kerror(ERR_BOOTINFO, "SET_PAGEDIR: %08X", dir);
	asm volatile("mov %0, %%cr3":: "b"(dir));
}

u32 *get_pagedir()
{
	u32 *dir;
	asm volatile("mov %%cr3, %0": "=a"(dir));
	return dir;
}

/**
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
 * Sets the paging flag in cr0.
 */
void disable_paging()
{
	u32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 &= ~0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

extern struct multiboot_module_tag *initrd;
/**
 * Creates a new page directory, and clears it. Thes creates a new page table,
 * and fills it with addresses starting at 0. Then it sets the page tables
 * first page table as the newly created one. Then it sets the page directory,
 * and enables paging.
 * 
 * @param eom end of memory
 * @see clear_pagedir
 * @see fill_pagetable
 * @see enable_paging
 */
void paging_init(u32 eom)
{
	firstframe = (u32 *)FRAMES_START;

	lastframe  = (u32 *)(eom & 0xFFFFF000);

	nframes = (u32)(eom - (u32)firstframe) / 0x1000;
	
	kerror(ERR_BOOTINFO, "  -> Clearing page frame table"); 
	
	u32 i = 0;
	for(; i < nframes; i += 32, frames[i] = 0);

	kerror(ERR_BOOTINFO, "  -> Clearing page directory");

	clear_pagedir(pagedir);

	kerror(ERR_BOOTINFO, "  -> Filling first 4 page tables");

	fill_pagetable((void *)init_tbls[0], 0x00000000);
	fill_pagetable((void *)init_tbls[1], 0x00400000);
	fill_pagetable((void *)init_tbls[2], 0x00800000);
	fill_pagetable((void *)init_tbls[3], 0x00C00000);

	kerror(ERR_BOOTINFO, "  -> Setting page directory entries");

	pagedir[0] = (u32)init_tbls[0] | 3;
	pagedir[1] = (u32)init_tbls[1] | 3;
	pagedir[2] = (u32)init_tbls[2] | 3;
	pagedir[3] = (u32)init_tbls[3] | 3;

	kerror(ERR_BOOTINFO, "  -> Setting page directory");

	kernel_cr3 = (u32)pagedir;
	set_pagedir(pagedir);

	kerror(ERR_BOOTINFO, "  -> Enabling paging");

	enable_paging();

	kerror(ERR_BOOTINFO, "  -> Initializing `malloc`");

	u32 alloc_mem = (u32)page_alloc(0x1000000 + 0x2000) & ~0xFFF; // 16 MB should be good for now
	init_alloc(alloc_mem, 0x1000000);

	block_page(0x00000000); // Catch NULL pointers and jumps

	kerror(ERR_BOOTINFO, "      -> %08X -> %08X (%08X)", alloc_mem, alloc_mem + 0x1000000, 0x1000000);
}


u32 *clone_kpagedir()
{
	u32 *pgd = (u32 *)(((ptr_t)kmalloc(sizeof(pagedir) + 0x1000) + 0x1000) & 0xFFFFF000);

	// kmalloc doesn't always gaive us mapped pages
	u32 i = 0;
	for(; i < sizeof(pagedir); i += 0x1000)
		map_page((void *)((u32)pgd + i), (void *)((u32)pgd + i), 0x03);

	memcpy(pgd, pagedir, sizeof(pagedir));
	return pgd;
}























/**
 * Get the first available memory hole of a certain size
 *
 * @param size the minimum size of the hole
 * @return the location of the memory hole
 */
static void *get_first_avail_hole(u32 size)
{
	int num_4k = 0;
	int size4k = size / 0x1000;
	if(size & 0x0FFF) size4k += 1;
	u32 i = 0;
	u32 base_i = 0;

	for(; i < nframes; i++)
	{
		if(!test_frame(i))
		{
			if(base_i == 0xFFFFFFFF) base_i = i;
			num_4k++;
			if(num_4k >= size4k)
			{
				u32 q = base_i;
				for(; q <= i; q++)
				{
					set_frame(q, 1);

					// BUG: This crashes while using kmalloc at some points...
					//
					//
					//
					//map_page((void *)((q * 0x1000) + (u32)firstframe), (void *)((q * 0x1000) + (u32)firstframe), 3);
				}
				return (void *)((base_i * 0x1000) + firstframe);
			}
		}
		else base_i = 0xFFFFFFFF;
	}
	kerror(ERR_SMERR, "Could not find %d KiB block", size4k * 4);
	return 0;
}

struct alloc_head //!< Header for allocated blocks
{
	u32 addr;  //!< Address of block
	u32 size;  //!< Size of block, including this header, AND padding
	u32 magic; //!< 0xA110CA1E
} __align(16);

/**
 * Allocate a region of memory to be used later.
 *
 * @param size size of the memory region
 * @return address of the memory region
 */
void *page_alloc(u32 size)
{
	int ints_en = interrupts_enabled();
	disable_interrupts();

	size += sizeof(struct alloc_head);
	u32 rsize = ((size & 0x0FFF) ? (size + 0x1000) : (size)) & 0xFFFFF000;

	void *base = get_first_avail_hole(rsize);
	if(!base)
	{
		// Error already reported, just return
		return NULL;
	}

	struct alloc_head *header = (struct alloc_head *)base;
	header->addr  = (u32)base + sizeof(struct alloc_head);
	header->size  = rsize;
	header->magic = 0xA110CA1E;

	if(ints_en) enable_interrupts();

	return (void *)header->addr;
}

/**
 * Free a region of memory that was allocated by malloc earlier.
 *
 * @param ptr pointer to memory region
 * @see malloc
 */
void page_free(void *ptr)
{
	if(!ptr)
	{
		kerror(ERR_SMERR, "Pointer given to `kfree` is null");
		return;
	}
	struct alloc_head *header = (struct alloc_head *)(ptr - sizeof(struct alloc_head));
	if((header->magic != 0xA110CA1E) || (header->addr != (u32)ptr))
	{
		kerror(ERR_SMERR, "Pointer given to `kfree` is invalid");
		return;
	}

	u32 i = 0;
	for(; i < header->size / 0x1000; i++)
		set_frame(i + (((u32)header - (u32)firstframe) / 0x1000), 0);

	// Invalidate header
	header->magic = 0;
	header->addr  = 0;
	header->size  = 0;
}
