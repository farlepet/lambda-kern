#include <types.h>
#include "paging.h"

#define N_ALLOCS 1024

struct alcent
{
	u32 valid;
	u32 used;
	u32 addr;
	u32 size;
};

struct alcent allocs[N_ALLOCS];

void *kmalloc(u32 sz);

void kfree(void *ptr);

void init_alloc(u32 base, u32 size);
