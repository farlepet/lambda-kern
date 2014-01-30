#include "alloc.h"
#include <string.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/atomic.h>

static lock_t alloc_lock = 0;

static struct alcent block_1[ALLOC_BLOCK];

static struct alcent *allocs[ALLOC_BLOCKS] = { block_1 };

/**
 * Add a memory block to the block list
 *
 * @param al block of memory to add
 */
static void add_alloc(struct alcent *al)
{
	int i, j = 0;

	// Search for a free allocation slot
	for(; j < ALLOC_BLOCKS; j++)
	{
		if(!allocs[j]) continue;
		for(i = 0; i < ALLOC_BLOCK; i++)
		{
			if(!allocs[j][i].valid) // Check if the allocation slot is in use
			{
				// Copy `al` to this allocation slot
				memcpy(&allocs[j][i], al, sizeof(struct alcent));
				return;
			}
		}
	}

	// This isn't a small error, but would YOU want to see this in a system log a  thousand times?
	kerror(ERR_INFO, "Could not add allocation structure to list, it's full!");
	return;
}

/**
 * Remove a used memory block (mark as free)
 *
 * @param idx index of the block to remove
 */
static void rm_alloc(int block, int idx)
{
	u32 addr = allocs[block][idx].addr;
	u32 size = allocs[block][idx].size;

	int i, j = 0;

	// See if this block immediately preceeds or proceeds any other block
	for(; j < ALLOC_BLOCKS; j++)
	{
		if(!allocs[j]) continue;
		for(i = 0; i < ALLOC_BLOCK; i++)
		{
			if(allocs[j][i].valid)
			{
				if((((allocs[j][i].addr + size - 1) == addr) ||
				(allocs[j][i].addr == (addr + size - 1))) && (!allocs[j][i].used))
				{
					allocs[j][i].size += size;
					allocs[block][idx].valid = 0;
					return;
				}
			}
		}
	}

	// No preceeding or proceeding block found
	allocs[block][idx].used = 0;
}

/**
 * Find the index of the smallest block of size `sz` or bigger
 *
 * @param sz amount of memory required
 * @returns index of block
 */
static u32 find_hole(u32 sz)
{
	u32 idx  = 0xFFFFFFFF;
	u32 size = 0xFFFFFFFF;
	
	int i, j = 0;

	// Find the smallest available block of free memory
	for(; j < ALLOC_BLOCKS; j++)
	{
		if(!allocs[j]) continue;
		for(i = 0; i < ALLOC_BLOCK; i++)
		{
			if(allocs[j][i].valid) // Is it a valid slot?
			{
				if(allocs[j][i].size > sz) // Is it big enough?
					if(allocs[j][i].size < size) // Is it smaller than the previously found block (if any)?
					{
						idx = (u16)i | j << 16;
						size = allocs[j][i].size;
					}
			}
		}
	}

	return idx;
}


static int empty_slots(int block)
{
	int fill = ALLOC_BLOCK;
	int i = 0;

	for(; i < ALLOC_BLOCK; i++)
		if(allocs[block][i].valid) fill--;

	return fill;
}

static int get_free_block()
{
	int i = 0;
	for(; i < ALLOC_BLOCKS; i++)
		if(allocs[i] == 0) return i;

	return ALLOC_BLOCKS;
}

/**
 * Allocate a block of memory
 *
 * @param sz size of the required block
 * @returns pointer to block
 */
void *kmalloc(u32 sz)
{
	kerror(ERR_DETAIL, "Allocating %d bytes of memory", sz);

	// We don't want two processes using the same memory block!
	lock(&alloc_lock);

	// Find the smallest memory block that we can use
	u32 idx = find_hole(sz);

	// Couldn't find one...
	if(idx == 0xFFFFFFFF) return 0;

	int block = idx >> 16;
	int index = idx & 0xFFFF;

	if(empty_slots(block) == 4) // Get ready ahead of time
	{
		u32 asz = ALLOC_BLOCK * sizeof(struct alcent);

		u32 idx = find_hole(asz);
		if(idx == 0xFFFFFFFF) kpanic("Could not create another allocation block!");

		int block = idx >> 16;
		int index = idx & 0xFFFF;

		if(allocs[block][index].size == asz)
		{
			allocs[block][index].used = 1;
			allocs[get_free_block()] = (struct alcent *)allocs[block][index].addr;
		}
		else
		{
			allocs[block][index].size -= asz;
			struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[block][index].addr, .size = asz };
			allocs[block][index].addr += asz;
			add_alloc(&ae);
			allocs[get_free_block()] = (struct alcent *)ae.addr;
		}
	}

	// If the previous block of code was used, we may have to reinitialize these
	idx = find_hole(sz);
	if(idx == 0xFFFFFFFF) return 0;
	block = idx >> 16;
	index = idx & 0xFFFF;


	if(allocs[block][index].size == sz)
	{
		allocs[block][index].used = 1;
		unlock(&alloc_lock);
		kerror(ERR_DETAIL, "  -> %08X W", allocs[block][index].addr);
		return (void *)allocs[block][index].addr;
	}

	allocs[block][index].size -= sz; // We are using part of this block

	struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[block][index].addr, .size = sz };

	allocs[block][index].addr += sz; // We don't want anything else using the allocated memory

	add_alloc(&ae); // We will just assume this worked, the worst that could happen is we can't `free` it (FIXME)

	// Let other processes allocate memory
	unlock(&alloc_lock);

	kerror(ERR_DETAIL, "  -> %08X P", ae.addr);
	return (void *)ae.addr;	
}

/**
 * Free an allocated block of memory
 *
 * @param ptr pointer to the previously allocated memory block
 */
void kfree(void *ptr)
{
	kerror(ERR_DETAIL, "Freeing %08X", ptr);

	lock(&alloc_lock);

	int i, j = 0;

	// Find the corresponding memory block
	for(; j < ALLOC_BLOCKS; j++)
	{
		if(!allocs[j]) continue;
		for(; i < ALLOC_BLOCK; i++)
			if(allocs[j][i].valid) // Is it valid?
				if(allocs[j][i].addr == (u32)ptr) // Is it the correct block?
					rm_alloc(j, i); // Free it!
	}

	unlock(&alloc_lock);
}

/**
 * Initialize allocation functions
 *
 * @param base base of the usable memory
 * @param size amount of usable memory
 */
void init_alloc(u32 base, u32 size)
{
	lock(&alloc_lock);

	struct alcent ae = { .valid = 1, .used = 0, .addr = base, .size = size };

	// Add a free allocation block encompasing all the usable memory
	add_alloc(&ae);

	unlock(&alloc_lock);
}
