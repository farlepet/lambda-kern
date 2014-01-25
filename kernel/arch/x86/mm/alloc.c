#include "alloc.h"
#include <string.h>
#include <err/error.h>
#include <proc/atomic.h>

lock_t alloc_lock = 0;

struct alcent allocs[N_ALLOCS]; // TODO: Make this an expandable array!!!!


void add_alloc(struct alcent *al)
{
	int i = 0;
	for(; i < N_ALLOCS; i++)
	{
		if(!allocs[i].valid)
		{
			memcpy(&allocs[i], al, sizeof(struct alcent));
			return;
		}
	}

	kerror(ERR_MEDERR, "Could not add allocation structure to list, it's full!");
	return;
}

void rm_alloc(int idx)
{
	u32 addr = allocs[idx].addr;
	u32 size = allocs[idx].size;

	int i = 0;
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

	// No preceeding block found
	allocs[idx].used = 0;
}

int find_hole(u32 sz)
{
	int idx = -1;
	u32 size = 0xFFFFFFFF;
	
	int i = 0;
	for(; i < N_ALLOCS; i++)
	{
		if(allocs[i].valid)
		{
			if(allocs[i].size > sz)
				if(allocs[i].size < size)
				{
					idx = i;
					size = allocs[i].size;
				}
		}
	}

	return idx;
}

void *kmalloc(u32 sz)
{
	kerror(ERR_DETAIL, "Allocating %d bytes of memory", sz);

	lock(&alloc_lock);

	int idx = find_hole(sz);

	if(idx < 0) return 0;

	if(allocs[idx].size == sz)
	{
		allocs[idx].used = 1;
		unlock(&alloc_lock);
		kerror(ERR_DETAIL, "  -> %08X W", allocs[idx].addr);
		return (void *)allocs[idx].addr;
	}

	allocs[idx].size -= sz;

	struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[idx].addr, .size = sz };

	allocs[idx].addr += sz;

	add_alloc(&ae); // We will just assume this worked, the worst that could happen is we can't `free` it (FIXME)

	unlock(&alloc_lock);

	kerror(ERR_DETAIL, "  -> %08X P", ae.addr);
	return (void *)ae.addr;	
}

void kfree(void *ptr)
{
	kerror(ERR_DETAIL, "Freeing %08X", ptr);

	lock(&alloc_lock);

	int i = 0;
	for(; i < N_ALLOCS; i++)
	{
		if(allocs[i].valid)
			if(allocs[i].addr == (u32)ptr)
			{
				rm_alloc(1);
			}
	}

	unlock(&alloc_lock);
}


void init_alloc(u32 base, u32 size)
{
	lock(&alloc_lock);

	struct alcent ae = { .valid = 1, .used = 0, .addr = base, .size = size };

	add_alloc(&ae);

	unlock(&alloc_lock);
}
