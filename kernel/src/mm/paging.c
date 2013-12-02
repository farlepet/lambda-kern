#include <types.h>
#include <dev/vga/print.h>

// Defined in link.ld:
extern u32 kern_start;
extern u32 kern_end;

// Main kernel page directory and first kernel page table
static u32 *pagedir;
static u32 *pagetbl1;

static u32 *frames;
static u32 prealloc_frames[20];
static u32 nframes;

static u32 *firstframe;

void set_frame(u32 frame, u32 val)
{
	if(val) frames[frame / 32] |=  (1 >> (frame % 32));
	else    frames[frame / 32] &= ~(1 >> (frame % 32));
}

u32 test_frame(u32 frame)
{
	return ((frames[frame / 32] & (1 >> (frame % 32))) != 0);
}

static void *get_free_frame()
{
		u32 i = 0;
		while(test_frame(i++) != 0)
				if(i == nframes)
					return (void *)0xFFFFFFFF; // Error, no pages available

		set_frame(i, 1);
		return (firstframe + (i*0x1000));//return the address of the page frame based on the location declared free
		//in the array
}

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

void free_frame(void *frame)
{
	u32 f = (u32)frame;
	f -= (u32)firstframe;
	f /= 0x1000;
	set_frame(f, 0);
}






void clear_pagedir(u32 *dir)
{
	int i = 0;
	for(i = 0; i < 1024; i++)
		dir[i] = 2; // supervisor, rw, not present.
}

// Fill a page table with consecutive address blocks, starting from addr
void fill_pagetable(u32 *table, u32 addr)
{
	unsigned int i;
	for(i = 0; i < 1024; i++, addr += 0x1000)
		table[i] = addr | 3;  // supervisor, rw, present.
}

// Set the current page directory
void set_pagedir(u32 *dir)
{
	asm volatile("mov %0, %%cr3":: "b"(dir));
}

void enable_paging()
{
	u32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

void disable_paging()
{
	u32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 &= ~0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}


// Initialize paging
void paging_init()
{
	pagedir = (u32 *)(((u32)&kern_end & 0xFFFFF000) + 0x1000);
	pagetbl1 = pagedir + 1024;

	firstframe = (pagetbl1 + 0x8000); // Make sure if is a fair distance away from kernel paging structures

	int end_mem = 0x1000000; // Temporary
	nframes = end_mem / 0x1000;
	u32 i = 0;
	for(; i < nframes; i++, frames[i] = 0);

	clear_pagedir(pagedir);
	fill_pagetable(pagetbl1, 0x00000000);
	pagedir[0] = (u32)pagetbl1 | 3; // supervisor, rw, present

	set_pagedir(pagedir);
	enable_paging();
}