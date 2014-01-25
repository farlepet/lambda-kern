#include "alloc.h"
#include <string.h>
#include <err/error.h>
#include <proc/atomic.h>

static lock_t alloc_lock = 0;

static struct alcent allocs[N_ALLOCS]; //!< Array of memory blocks (TODO: Make this an expandable array!)

/**
 * Add a memory block to the block list
 *
 * @param al block of memory to add
 */
static void add_alloc(struct alcent *al)
{
	int i = 0;

	// Search for a free allocation slot
	for(; i < N_ALLOCS; i++)
	{
		if(!allocs[i].valid) // Check if the allocation slot is in use
		{
			// Copy `al` to this allocation slot
			memcpy(&allocs[i], al, sizeof(struct alcent));
			return;
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
static void rm_alloc(int idx)
{
	u32 addr = allocs[idx].addr;
	u32 size = allocs[idx].size;

	int i = 0;

	// See if this block immediately preceeds or proceeds any other block
	for(; i < N_ALLOCS; i++)
	{
		if(allocs[i].valid)
		{
			if((((allocs[i].addr + size - 1) == addr) ||
			(allocs[i].addr == (addr + size - 1))) && (!allocs[i].used))
			{
				allocs[i].size += size;
				allocs[idx].valid = 0;
				return;
			}
		}
	}

	// No preceeding or proceeding block found
	allocs[idx].used = 0;
}

/**
 * Find the index of the smallest block of size `sz` or bigger
 *
 * @param sz amount of memory required
 * @returns index of block
 */
static int find_hole(u32 sz)
{
	int idx = -1;
	u32 size = 0xFFFFFFFF;
	
	int i = 0;

	// Find the smallest available block of free memory
	for(; i < N_ALLOCS; i++)
	{
		if(allocs[i].valid) // Is it a valid slot?
		{
			if(allocs[i].size > sz) // Is it big enough?
				if(allocs[i].size < size) // Is it smaller than the previously found block (if any)?
				{
					idx = i;
					size = allocs[i].size;
				}
		}
	}

	return idx;
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
	int idx = find_hole(sz);

	// Couldn't find one...
	if(idx < 0) return 0;

	if(allocs[idx].size == sz)
	{
		allocs[idx].used = 1;
		unlock(&alloc_lock);
		kerror(ERR_DETAIL, "  -> %08X W", allocs[idx].addr);
		return (void *)allocs[idx].addr;
	}

	allocs[idx].size -= sz; // We are using part of this block

	struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[idx].addr, .size = sz };

	allocs[idx].addr += sz; // We don't want anything else using the allocated memory

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

	int i = 0;

	// Find the corresponding memory block
	for(; i < N_ALLOCS; i++)
		if(allocs[i].valid) // Is it valid?
			if(allocs[i].addr == (u32)ptr) // Is it the correct block?
				rm_alloc(i); // Free it!

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
