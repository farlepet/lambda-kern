#include <lambda/export.h>
#include <string.h>
#include <mm/mm.h>
#include <mm/alloc.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/mtask.h>
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
static void rm_alloc(int block, int idx) {
	uint32_t addr = allocs[block][idx].addr;
	uint32_t size = allocs[block][idx].size;

	int i, j = 0;

	// See if this block immediately preceeds or proceeds any other block
	for(; j < ALLOC_BLOCKS; j++) {
		if(!allocs[j]) {
			continue;
		}
		for(i = 0; i < ALLOC_BLOCK; i++) {
			if(allocs[j][i].valid) {
				if((((allocs[j][i].addr + size) == addr)  ||
				    (allocs[j][i].addr == (addr + size))) &&
				   (!allocs[j][i].used)) {
					kdebug(DEBUGSRC_MM, "  -> Merging free'd block (%08X,%d) with (%08X,%d)", addr, size, allocs[j][i].addr, allocs[j][i].size);
					allocs[j][i].size += size;
					if(allocs[j][i].addr > addr) {
						allocs[j][i].addr = addr;
					}
					allocs[block][idx].valid = 0;
					return;
				}
			}
		}
	}

	kdebug(DEBUGSRC_MM, "  -> Marking free'd block (%08X,%d)", addr, size);
	// No preceeding or proceeding block found
	allocs[block][idx].used = 0;
}

/**
 * Find the index of the smallest block of size `sz` or bigger
 * 
 * @todo There are more efficient ways to find a hole with close alignment and minimal waste
 *
 * @param sz amount of memory required
 * @returns index of block
 */
static uint32_t _find_hole(size_t sz, size_t align)
{
	uint32_t idx  = 0xFFFFFFFF;
	uint32_t size = 0xFFFFFFFF;

	int i, j = 0;

	// Find the smallest available block of free memory
	for(; j < ALLOC_BLOCKS; j++) {
		if(!allocs[j]) {
			continue;
		}
		for(i = 0; i < ALLOC_BLOCK; i++) {
			if(allocs[j][i].valid && !allocs[j][i].used) { // Is it a valid and unused slot?
				uint32_t off = align - (allocs[j][i].addr % align);
				if(off == align) {
					off = 0;
				}
				if((allocs[j][i].size - off) >= sz) { // Is it big enough, and allow the required alignment?
					if(allocs[j][i].size < size) { // Is it smaller than the previously found block (if any)?
						idx = (uint16_t)i | j << 16;
						size = allocs[j][i].size;
					}
				}
			}
		}
	}

	return idx;
}


static int _empty_slots(int block)
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

static void _alloc_new_block(void) {
	kdebug(DEBUGSRC_MM, "_alloc_new_block");
	uint32_t asz = ALLOC_BLOCK * sizeof(struct alcent);

	uint32_t idx = _find_hole(asz, 1);
	if(idx == 0xFFFFFFFF) kpanic("Could not create another allocation block!");

	uint16_t block = idx >> 16;
	uint16_t index = idx & 0xFFFF;

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


void *kamalloc(size_t sz, size_t align) {
	if(!align || !sz) {
		return NULL;
	}

	kdebug(DEBUGSRC_MM, "Allocating %d bytes of memory with alignment %d", sz, align);

	// We don't want two processes using the same memory block!
	lock(&alloc_lock);

	// Find the smallest memory block that we can use
	uint32_t idx = _find_hole(sz, align);

	// Couldn't find one...
	if(idx == 0xFFFFFFFF) {
		unlock(&alloc_lock);
		return 0;
	}
	int block = idx >> 16;
	int index;

	/* TODO: This should be checking all free slots, not just in the returned block */
	if(_empty_slots(block) == 4) {
		_alloc_new_block();
	}

	// If the previous block of code was used, we may have to reinitialize these
	idx = _find_hole(sz, align);
	if(idx == 0xFFFFFFFF) {
		unlock(&alloc_lock);
		return 0;
	}
	block = idx >> 16;
	index = idx & 0xFFFF;


	if(allocs[block][index].size == sz) {
		allocs[block][index].used = 1;
		unlock(&alloc_lock);
		kerror(ERR_DETAIL, "  -> %08X WH", allocs[block][index].addr);
		return (void *)allocs[block][index].addr;
	}
	
	struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[block][index].addr, .size = sz };

	if(allocs[block][index].addr % align) {
		/* Block isn't perfectly aligned */
		size_t correction = align - (allocs[block][index].addr % align);
		ae.addr = allocs[block][index].addr + correction;
		add_alloc(&ae);
		if(allocs[block][index].size > (sz + correction)) {
			/* Block also extends beyond needed space */
			struct alcent fe = {.valid = 1, .used = 0,
			                    .addr = allocs[block][index].addr + (sz + correction),
								.size = allocs[block][index].size - (sz + correction) };
			add_alloc(&fe);
		}
		allocs[block][index].size = correction;
		kdebug(DEBUGSRC_MM, "  -> %08X PU", ae.addr);
	} else {
		/* Block is aligned, but has extra space at the end */
		allocs[block][index].size -= sz;
		allocs[block][index].addr += sz;
		kdebug(DEBUGSRC_MM, "  -> %08X PA", ae.addr);
	}

	add_alloc(&ae);

	// Let other processes allocate memory
	unlock(&alloc_lock);

	return (void *)ae.addr;
}
EXPORT_FUNC(kamalloc);

void *kmalloc(size_t sz) {
	return kamalloc(sz, 1);
}
EXPORT_FUNC(kmalloc);

void *kmamalloc(size_t sz, size_t align) {
	if(sz % align) {
		sz += align - (sz % align);
	}
	return kamalloc(sz, align);
}
EXPORT_FUNC(kmamalloc);

void kfree(void *ptr)
{
	kdebug(DEBUGSRC_MM, "Freeing %08X", ptr);

	lock(&alloc_lock);

	int i, j = 0;

	// Find the corresponding memory block
	for(; j < ALLOC_BLOCKS; j++)
	{
		if(!allocs[j]) continue;
		for(i = 0; i < ALLOC_BLOCK; i++)
			if(allocs[j][i].valid) // Is it valid?
				if(allocs[j][i].addr == (uint32_t)ptr) // Is it the correct block?
					rm_alloc(j, i); // Free it!
	}

	unlock(&alloc_lock);
}
EXPORT_FUNC(kfree);


void *malloc(size_t sz) {
	void *ret = kmalloc(sz);
	if(!ret) {
		return NULL;
	}
	struct kproc *proc = mtask_get_current_task();
	if(proc) {
		/* NOTE: Assuming one-to-one mapping. */
		mm_proc_mmap_add(proc, (uintptr_t)ret, (uintptr_t)ret, sz);
	}

	return ret;
}

void free(void *ptr) {
	struct kproc *proc = mtask_get_current_task();
	if(proc) {
		/* NOTE: Assuming allocation table is using physical addresses. */
		mm_proc_mmap_remove_phys(proc, (uintptr_t)ptr);
	}
	kfree(ptr);
}


/**
 * Initialize allocation functions
 *
 * @param base base of the usable memory
 * @param size amount of usable memory
 */
void init_alloc(uint32_t base, uint32_t size)
{
	// Initialized before multitasking, this shouldn't be needed
	//lock(&alloc_lock);

	struct alcent ae = { .valid = 1, .used = 0, .addr = base, .size = size };

	// Add a free allocation block encompasing all the usable memory
	add_alloc(&ae);

	unlock(&alloc_lock);
}