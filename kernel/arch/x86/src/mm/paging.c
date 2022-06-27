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

#define N_INIT_TABLES     1 /* 1 * 1024 * 1024 = 1 MiB */

static uint32_t pagedir[PGDIR_ENTRIES]                  __align(0x1000); //!< Main kernel pagedirectory
static uint32_t init_tbls[N_INIT_TABLES][PGTBL_ENTRIES] __align(0x1000); //!< First n page tables

uint32_t kernel_cr3;        //!< Page directory used by the kernel

int page_present(uint32_t virtaddr) {
	uint32_t *pgdir = get_pagedir();

	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

    if(pgdir[pdindex] & 0x01) {
		return ((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex] & 0x01;
	}

	return 0;
}

uint32_t pgdir_get_page_entry(const uint32_t *pgdir, const void *virtaddr) {
	uint32_t pdindex = (uint32_t)virtaddr >> 22;
	uint32_t ptindex = (uint32_t)virtaddr >> 12 & 0x03FF;

	if(pgdir[pdindex] & 0x01) {
		return ((uint32_t *)(pgdir[pdindex] & 0xFFFFF000))[ptindex];
	}
	return 0;
}

uint32_t get_page_entry(const void *virtaddr) {
	uint32_t *pgdir = get_pagedir();
	return pgdir_get_page_entry(pgdir, virtaddr);
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
	if (som % PAGE_SZ) {
		som = (som + PAGE_SZ) & 0xFFFFF000;
	}

	kdebug(DEBUGSRC_MM, ERR_INFO, "  -> Clearing page directory");

	memset(pagedir, 0, sizeof(pagedir));

	kdebug(DEBUGSRC_MM, ERR_INFO, "  -> Filling first 4 page tables");

	/* This is temporary, it will later by updated based on the kernel memory map, and the memory given to alloc */
	for(size_t i = 0; i < N_INIT_TABLES; i++) {
		// Identity-map first N_INIT_TABLES * 4 MiB for kernel use. (flags: present, writable)
		fill_pagetable((void *)init_tbls[i], i * (PAGE_SZ * PGTBL_ENTRIES), 0x003);
		pagedir[i] = (uint32_t)init_tbls[i] | 3;
	}

	kdebug(DEBUGSRC_MM, ERR_INFO, "  -> Setting page directory");

	kernel_cr3 = (uint32_t)pagedir;
	set_pagedir(pagedir);

	kdebug(DEBUGSRC_MM, ERR_INFO, "  -> Enabling paging");

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

	map_page(0x00000000, 0x00000000, 0); // Catch NULL pointers and jumps

	kdebug(DEBUGSRC_MM, ERR_INFO, "  -> Initializing allocation system");
	init_alloc(som, eom - som);

	kdebug(DEBUGSRC_MM, ERR_INFO, "      -> %08X -> %08X (%08X)", som, eom, (eom - som));
}
