#include <arch/intr/int.h>
#include <arch/mm/paging.h>
#include <arch/mm/mem.h>

#include <multiboot.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>
#include <types.h>
#include <video.h>

static uint32_t pagedir[1024]      __align(0x1000); //!< Main kernel pagedirectory
static uint32_t init_tbls[4][1024] __align(0x1000); //!< First 4 page tables
static uint32_t frames[0x10000]    __align(0x1000); //!< Table stating which frames are available, takes up 256KiB
static uint32_t prealloc_frames[20];                //!< 20 frames that are free, used by alloc_frame
static uint32_t nframes;                            //!< Number of frames in the table

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
	//kerror(ERR_BOOTINFO, "  set_frame(%08X, %08X)", frame, val);
	if(val == 1)
	{
		frames[frame / 32] |=  (1 << (frame % 32));
	}
	else if(val == 0) frames[frame / 32] &= ~(1 << (frame % 32));
	else if(val == 0xFFFFFFFF) // BLOCK this page from use
	{
		frames[frame / 32] |= (1 << (frame % 32));
		uint32_t addr = (((frame * 0x1000) + (uint32_t)firstframe) & 0xFFFFF000);
		uint32_t pdindex = addr >> 22;
		uint32_t ptindex = addr >> 12 & 0x03FF;
		((uint32_t *)(pagedir[pdindex] & 0xFFFFF000))[ptindex] = 0x00000000; // Invalidate the page
	}
	else kerror(ERR_MEDERR, "invalid value to set_frame: %d", val);
}

void block_page(uint32_t page) {
	set_frame((page - (uint32_t)firstframe) / 0x1000, 0xFFFFFFFF);
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

	map_page(firstframe + (i*0x1000), firstframe + (i*0x1000), 3); // Make sure it is mapped
		
	return (firstframe + (i*0x1000));
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
	kdebug(DEBUGSRC_MM, "Mapping %8X to %8X (%03X) in %8X", virtualaddr, physaddr, flags, pgdir);

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
	int i = 0;
	for(i = 0; i < 1024; i++) {
	//	kerror(ERR_INFO, "      -> PDIRENT %X", i);
		dir[i] = 2; // supervisor, rw, not present.
	}
}

void fill_pagetable(uint32_t *table, uint32_t addr, uint32_t flags) {
	uint32_t i;
	for(i = 0; i < 1024; i++, addr += 0x1000)
		table[i] = addr | (flags & 0xFFF);
}

void set_pagedir(uint32_t *dir) {
	if(!dir) {
		kpanic("Attempted to set pagedir pointer to NULL!");
	}

	kerror(ERR_BOOTINFO, "SET_PAGEDIR: %08X", dir);
	asm volatile("mov %0, %%cr3":: "b"(dir));
}

uint32_t *get_pagedir() {
	uint32_t *dir;
	asm volatile("mov %%cr3, %0": "=a"(dir));
	return dir;
}

void enable_paging() {
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

void disable_paging() {
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 &= ~0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

/**
 * Enable the use of global page flag.
 */
static void enable_global_pages() {
	uint32_t cr4;
	asm volatile("mov %%cr4, %0": "=b"(cr4));
	cr4 |= (1UL << 7);
	asm volatile("mov %0, %%cr4":: "b"(cr4));
}

/**
 * Disable the use of global page flag.
 */
__unused
static void disable_global_pages() {
	uint32_t cr4;
	asm volatile("mov %%cr4, %0": "=b"(cr4));
	cr4 &= ~(1UL << 7);
	asm volatile("mov %0, %%cr4":: "b"(cr4));
}

extern struct multiboot_module_tag *initrd;

void paging_init(uint32_t som, uint32_t eom) {
	firstframe = (uint32_t *)som;

	lastframe  = (uint32_t *)(eom & 0xFFFFF000);

	nframes = (uint32_t)(eom - (uint32_t)firstframe) / 0x1000;
	
	kerror(ERR_BOOTINFO, "  -> Clearing page frame table"); 
	
	uint32_t i = 0;
	for(; i < nframes; i += 32, frames[i] = 0);

	kerror(ERR_BOOTINFO, "  -> Clearing page directory");

	clear_pagedir(pagedir);

	kerror(ERR_BOOTINFO, "  -> Filling first 4 page tables");

	/* @todo Base this on the actual used memory, rather than automatically taking up 16 MiB */
	// Identity-map first 16 MiB for kernel use. (flags: present, writable, global)
	fill_pagetable((void *)init_tbls[0], 0x00000000, 0x103);
	fill_pagetable((void *)init_tbls[1], 0x00400000, 0x103);
	fill_pagetable((void *)init_tbls[2], 0x00800000, 0x103);
	fill_pagetable((void *)init_tbls[3], 0x00C00000, 0x103);

	kerror(ERR_BOOTINFO, "  -> Setting page directory entries");

	pagedir[0] = (uint32_t)init_tbls[0] | 3;
	pagedir[1] = (uint32_t)init_tbls[1] | 3;
	pagedir[2] = (uint32_t)init_tbls[2] | 3;
	pagedir[3] = (uint32_t)init_tbls[3] | 3;

	kerror(ERR_BOOTINFO, "  -> Setting page directory");

	kernel_cr3 = (uint32_t)pagedir;
	set_pagedir(pagedir);

	kerror(ERR_BOOTINFO, "  -> Enabling paging");

	enable_paging();
	enable_global_pages();

	block_page(0x00000000); // Catch NULL pointers and jumps

	kerror(ERR_BOOTINFO, "  -> Initializing `malloc`");
	kerror(ERR_BOOTINFO, "      -> Allocating page frames");
	uint32_t alloc_mem = (uint32_t)page_alloc(0x1000000 + 0x2000) & ~0xFFF; // 16 MB should be good for now

	kerror(ERR_BOOTINFO, "      -> Initializing allocation system");
	init_alloc(alloc_mem, 0x1000000);

	kerror(ERR_BOOTINFO, "      -> %08X -> %08X (%08X)", alloc_mem, alloc_mem + 0x1000000, 0x1000000);
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
	kerror(ERR_SMERR, "Could not find %d KiB block", size4k * 4);
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
		kerror(ERR_SMERR, "Pointer given to `kfree` is null");
		return;
	}
	struct alloc_head *header = (struct alloc_head *)(ptr - sizeof(struct alloc_head));
	if((header->magic != 0xA110CA1E) || (header->addr != (uint32_t)ptr))
	{
		kerror(ERR_SMERR, "Pointer given to `kfree` is invalid");
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
