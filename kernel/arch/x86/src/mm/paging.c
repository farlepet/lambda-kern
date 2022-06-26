#include <arch/types/mmu.h>
#include <arch/intr/int.h>
#include <arch/mm/paging.h>
#include <arch/mm/mem.h>
#include <arch/registers.h>

#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>
#include <types.h>
#include <video.h>

#define N_INIT_TABLES     1
#define N_FRAMES          0x8000 /* 4 GiB / (32 * 4 KiB) = 0x8000 */
#define N_PREALLOC_FRAMES 20

static uint32_t pagedir[PGDIR_ENTRIES]                  __align(0x1000); //!< Main kernel pagedirectory
static uint32_t init_tbls[N_INIT_TABLES][PGTBL_ENTRIES] __align(0x1000); //!< First n page tables
static uint32_t frames[N_FRAMES];                                        //!< Table stating which frames are available
static uint32_t prealloc_frames[N_PREALLOC_FRAMES];                      //!< 20 frames that are free, used by alloc_frame
static uint32_t nframes;                                                 //!< Number of frames in the table

uint32_t kernel_cr3;        //!< Page directory used by the kernel

uint32_t *firstframe;       //!< The location of the first page frame
static uint32_t *lastframe; //!< The location of the last page frame

/**
 * Sets the current frame to be used or unused, depending on `val`.
 * 
 * @param frame the frame to be set
 * @param val wether the frame is used or unused
 */
void set_frame(uint32_t frame, uint32_t val) {
	switch(val) {
		case 0: /* Free frame */
			frames[frame / 32] &= ~(1 << (frame % 32));
			break;
		case 1: /* Use frame */
			frames[frame / 32] |=  (1 << (frame % 32));
			break;
		case 0xFFFFFFFF: { /* BLOCK this page from use */
			frames[frame / 32] |= (1 << (frame % 32));
			uint32_t addr = (((frame * 0x1000) + (uint32_t)firstframe) & 0xFFFFF000);
			uint32_t pdindex = addr >> 22;
			uint32_t ptindex = addr >> 12 & 0x03FF;
			((uint32_t *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] = 0x00000000; // Invalidate the page
		} break;
		default:
			kpanic("Invalid value to set_frame: %d", val);
	}
}

void block_page(uint32_t page) {
	set_frame((page - (uint32_t)firstframe) / PAGE_SZ, 0xFFFFFFFF);
}

/**
 * Checks if the frame is not being used.
 * 
 * @param frame the frame to be tested
 */
static uint32_t test_frame(uint32_t frame) {
	return ((frames[frame / 32] & (1 << (frame % 32))) != 0);
}

/**
 * Look through the pages, and see if any are available.
 */
static void *get_free_frame() {
	uint32_t i = 0;
	while(test_frame(i) != 0) {
		if(frames[i/32] == 0xFFFFFFFF) {
			i += 32;
			continue;
		}
		if(i == nframes)
			return (void *)0xFFFFFFFF; // Error, no pages available
		i++;
	}

	set_frame(i, 1);

	map_page(firstframe + (i*PAGE_SZ), firstframe + (i*PAGE_SZ), 3); // Make sure it is mapped
		
	return (firstframe + (i*PAGE_SZ));
}

/**
 * Allocates a page frame then returns its address
 */
void *alloc_frame() {
		static uint32_t pframe = 20;
 
		if(pframe == 20) {
			int i = 0;
			for(; i < 20; i++) {
				prealloc_frames[i] = (uint32_t)get_free_frame();
				if(prealloc_frames[i] == 0xFFFFFFFF) return (void *)0xFFFFFFFF; // TODO: Add some type of handling system here
			}
			pframe = 0;
		}
		return (void *)prealloc_frames[pframe++];
}

void *get_phys_page(void *virtaddr) {
	uint32_t *pgdir = get_pagedir();

	void *off = (void *)((uint32_t)virtaddr & 0x00000FFF);
	virtaddr = (void *)((uint32_t)virtaddr & 0xFFFFF000);

	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

	if(pgdir[pdindex] & 0x01) {
		if(((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] & 0x01)
			return (void *)((((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] & 0xFFFFF000) | (uint32_t)off);
		else
			return NULL;
	}

	else return NULL;
}

int page_present(uint32_t virtaddr) {
	uint32_t *pgdir = get_pagedir();

	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

    if(pgdir[pdindex] & 0x01) {
		return ((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] & 0x01;
	}

	return 0;
}

uint32_t get_page_entry(const void *virtaddr) {
	uint32_t *pgdir = get_pagedir();

	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

	if(pgdir[pdindex] & 0x01) {
		return ((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex];
	}
	return 0;
}


uint32_t pgdir_get_page_entry(uint32_t *pgdir, const void *virtaddr) {
	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

	if(pgdir[pdindex] & 0x01) {
		return ((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex];
	}
	return 0;
}

uint32_t pgdir_get_phys_addr(uint32_t *pgdir, const void *virtaddr) {
	return (pgdir_get_page_entry(pgdir, virtaddr) & ~(0xFFF)) | ((uint32_t)virtaddr & 0xFFF);
}

uint32_t pgdir_get_page_table(uint32_t *pgdir, const void *virtaddr) {
	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	
	return pgdir[pdindex];
}

void map_page(void *physaddr, void *virtualaddr, uint32_t flags) {
	uint32_t *pgdir = get_pagedir();
	
	if(flags & PAGE_TABLE_FLAG_GLOBAL && pgdir != pagedir) {
		/* Include global pages in the kernel source-of-truth page directory */
		pgdir_map_page(pagedir, physaddr, virtualaddr, flags);
	
	}

	pgdir_map_page(pgdir, physaddr, virtualaddr, flags);
	__invlpg(virtualaddr);
}

void pgdir_map_page(uint32_t *pgdir, void *physaddr, void *virtualaddr, uint32_t flags) {
	virtualaddr = (void *)((uint32_t)virtualaddr & 0xFFFFF000);
	physaddr    = (void *)((uint32_t)physaddr    & 0xFFFFF000);

	uint32_t pdindex = (uint32_t)virtualaddr >> 22;
	uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;

	if(!(pgdir[pdindex] & 0x01)) {
		pgdir[pdindex] = (uint32_t)kamalloc(0x1000, 0x1000) | 0x03;

		int i = 0;
		for(; i < 1024; i++)
			((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[i] = 0x00000000;
	}

	pgdir[pdindex] |= flags & 0x04;
	((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] = ((uint32_t)physaddr) | (flags & 0xFFF) | 0x01;	
}

void free_frame(void *frame) {
	set_frame(((uint32_t)frame - (uint32_t)firstframe) / 0x1000, 0);
}



void clear_pagedir(uint32_t *dir) {
	for(uint32_t i = 0; i < PGDIR_ENTRIES; i++) {
		dir[i] = 2; // supervisor, rw, not present.
	}
}

void fill_pagetable(uint32_t *table, uint32_t addr, uint32_t flags) {
	for(uint32_t i = 0; i < PGTBL_ENTRIES; i++, addr += PAGE_SZ) {
		table[i] = addr | (flags & 0xFFF);
	}
}

void set_pagedir(uint32_t *dir) {
	if(!dir) {
		kpanic("Attempted to set pagedir pointer to NULL!");
	}

	kdebug(DEBUGSRC_MM, ERR_TRACE, "SET_PAGEDIR: %08X", dir);
	asm volatile("mov %0, %%cr3":: "b"(dir));
}

uint32_t *get_pagedir() {
	uint32_t *dir;
	asm volatile("mov %%cr3, %0": "=a"(dir));
	return dir;
}

void enable_paging() {
	uint32_t cr0 = register_cr0_read();
	cr0 |= CR0_FLAG_PG;
	register_cr0_write(cr0);
}

void disable_paging() {
	uint32_t cr0 = register_cr0_read();
	cr0 &= ~CR0_FLAG_PG;
	register_cr0_write(cr0);
}

void paging_init(uint32_t som, uint32_t eom) {
	if(som % PAGE_SZ) {
		firstframe = (uint32_t *)((som + PAGE_SZ) & 0xFFFFF000);
	} else {
		firstframe = (uint32_t *)som;
	}
	lastframe  = (uint32_t *)(eom & 0xFFFFF000);
	nframes    = (uint32_t)(eom - (uint32_t)firstframe) / PAGE_SZ;
	
	if(nframes > N_FRAMES) {
		kpanic("paging_init: Number of required frames exceeds that allocated! %d > %d", nframes, N_FRAMES);
	}

	kerror(ERR_INFO, "  -> Clearing page frame table"); 
	
	for(size_t i = 0; i < (nframes / 32); i++, frames[i] = 0) {
		frames[i] = 0;
	}

	kerror(ERR_INFO, "  -> Clearing page directory");

	clear_pagedir(pagedir);

	kerror(ERR_INFO, "  -> Filling first 4 page tables");

	/* @todo Base this on the actual used memory */
	for(size_t i = 0; i < N_INIT_TABLES; i++) {
		// Identity-map first N_INIT_TABLES * 4 MiB for kernel use. (flags: present, writable)
		fill_pagetable((void *)init_tbls[i], i * (PAGE_SZ * PGTBL_ENTRIES), 0x003);
		pagedir[i] = (uint32_t)init_tbls[i] | 3;
	}

	kerror(ERR_INFO, "  -> Setting page directory");

	kernel_cr3 = (uint32_t)pagedir;
	set_pagedir(pagedir);

	kerror(ERR_INFO, "  -> Enabling paging");

	uint32_t tmp;

	/* Enable write protect in supervisor mode */
	tmp = register_cr0_read();
	tmp |= CR0_FLAG_WP;
	register_cr0_write(tmp);

	/* Enable global pages */
	tmp = register_cr4_read();
	tmp |= CR4_FLAG_PGE;
	register_cr4_write(tmp);

	enable_paging();

	block_page(0x00000000); // Catch NULL pointers and jumps

	kerror(ERR_INFO, "  -> Initializing `malloc`");
	kerror(ERR_INFO, "      -> Allocating page frames");
#define ALLOC_MEM_SZ 0x1000000 /* TODO: Make this dynamic */
	uint32_t alloc_mem = (uint32_t)page_alloc(ALLOC_MEM_SZ + PAGE_SZ); // 16 MB should be good for now
	if(alloc_mem % PAGE_SZ) {
		alloc_mem = (alloc_mem + PAGE_SZ) & 0xFFFFF000;
	}

	kerror(ERR_INFO, "      -> Initializing allocation system");
	init_alloc(alloc_mem, ALLOC_MEM_SZ);

	kerror(ERR_INFO, "      -> %08X -> %08X (%08X)", alloc_mem, alloc_mem + ALLOC_MEM_SZ, ALLOC_MEM_SZ);
}


uint32_t *clone_kpagedir() {
	return clone_pagedir(pagedir);
}

uint32_t *clone_pagedir(uint32_t *pgdir) {
	// TODO: Make 1024 an actual constant:
	uint32_t *pgd = kamalloc(1024 * sizeof(uint32_t), 0x1000);

	// kmalloc doesn't always gaive us mapped pages
	uint32_t i = 0;
	for(; i < sizeof(pgdir); i += 0x1000)
		map_page((void *)((uint32_t)pgd + i), (void *)((uint32_t)pgd + i), 0x03);

	memcpy(pgd, pgdir, 1024 * sizeof(uint32_t));
	return pgd;
}

uint32_t *clone_pagedir_full(uint32_t *pgdir) {
	// Count how many tables we need to allocate:
	size_t n_tables = 0;
	for(size_t i = 0; i < 1024; i++) {
		if(pgdir[i] & 1) {
			n_tables++;
		}
	}

	// TODO: Make 1024 an actual constant:
	uint32_t *pgd = kamalloc((0x1000 + (0x1000 * n_tables)), 0x1000);

	// kmalloc doesn't always gaive us mapped pages
	for(uint32_t i = 0; i < sizeof(pgdir); i += 0x1000)
		map_page((void *)((uint32_t)pgd + i), (void *)((uint32_t)pgd + i), 0x03);

	memcpy(pgd, pgdir, 1024 * sizeof(uint32_t));

	// TODO: Posible don't copy pages marked as global?

	size_t idx = 1;
	for(size_t i = 0; i < 1024; i++) {
		if(pgd[i] & 1) {
			// Modify entry and copy table:
			pgd[i] = (pgd[i] & 0xFFF) | ((uint32_t)pgd + (0x1000 * idx));
			memcpy((void *)(pgd[i] & 0xFFFFF000), (void *)(pgdir[i] & 0xFFFFF000), 0x1000);
			idx++;
		}
	}

	return pgd;
}




















/**
 * Get the first available memory hole of a certain size
 *
 * @param size the minimum size of the hole
 * @return the location of the memory hole
 */
static void *get_first_avail_hole(uint32_t size) {
	int num_4k = 0;
	int size4k = size / 0x1000;
	if(size & 0x0FFF) size4k += 1;
	uint32_t i = 0;
	uint32_t base_i = 0;

	for(; i < nframes; i++) {
		if(!test_frame(i)) {
			if(base_i == 0xFFFFFFFF) base_i = i;
			num_4k++;
			if(num_4k >= size4k) {
				uint32_t q = base_i;
				for(; q <= i; q++) {
					set_frame(q, 1);
				}
				return (void *)((base_i * 0x1000) + firstframe);
			}
		}
		else base_i = 0xFFFFFFFF;
	}
	kerror(ERR_ERROR, "Could not find %d KiB block", size4k * 4);
	return 0;
}

struct alloc_head //!< Header for allocated blocks
{
	uint32_t addr;  //!< Address of block
	uint32_t size;  //!< Size of block, including this header, AND padding
	uint32_t magic; //!< 0xA110CA1E
};

/**
 * Allocate a region of memory to be used later.
 *
 * @param size size of the memory region
 * @return address of the memory region
 */
void *page_alloc(uint32_t size)
{
	int ints_en = interrupts_enabled();
	disable_interrupts();

	size += sizeof(struct alloc_head);
	uint32_t rsize = ((size & 0x0FFF) ? (size + 0x1000) : (size)) & 0xFFFFF000;

	void *base = get_first_avail_hole(rsize);
	if(!base)
	{
		// Error already reported, just return
		return NULL;
	}

	struct alloc_head *header = (struct alloc_head *)base;
	header->addr  = (uint32_t)base + sizeof(struct alloc_head);
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
		kerror(ERR_WARN, "Pointer given to `kfree` is null");
		return;
	}
	struct alloc_head *header = (struct alloc_head *)(ptr - sizeof(struct alloc_head));
	if((header->magic != 0xA110CA1E) || (header->addr != (uint32_t)ptr))
	{
		kerror(ERR_WARN, "Pointer given to `kfree` is invalid");
		return;
	}

	uint32_t i = 0;
	for(; i < header->size / 0x1000; i++)
		set_frame(i + (((uint32_t)header - (uint32_t)firstframe) / 0x1000), 0);

	// Invalidate header
	header->magic = 0;
	header->addr  = 0;
	header->size  = 0;
}
