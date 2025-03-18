#include <lambda/export.h>
#include <string.h>
#include <mm/mm.h>
#include <mm/alloc.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/mtask.h>
#include <proc/atomic/lock.h>

static lock_t alloc_lock = 0;

static struct alcent block_1[ALLOC_BLOCK];

static struct alcent *allocs[ALLOC_BLOCKS] = { block_1 };

#define MIN_ALIGN 4

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

    kpanic("Could not add allocation structure to list, it's full!");
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
                    kdebug(DEBUGSRC_MM, ERR_TRACE, "  -> Merging free'd block (%08X,%d) with (%08X,%d)", addr, size, allocs[j][i].addr, allocs[j][i].size);
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

    kdebug(DEBUGSRC_MM, ERR_TRACE, "  -> Marking free'd block (%08X,%d)", addr, size);
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
                if(allocs[j][i].size >= (sz + off)) { // Is it big enough, and allow the required alignment?
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
    kdebug(DEBUGSRC_MM, ERR_TRACE, "_alloc_new_block");
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

    kdebug(DEBUGSRC_MM, ERR_TRACE, "Allocating %d bytes of memory with alignment %d", sz, align);
    /* NOTE: The following line caused ARMv7 to get stuck in `alloc_get_used()`
     * in the kterm task. */
    //kdebug(DEBUGSRC_MM, ERR_TRACE, "  Used: %u, Free: %u", alloc_get_used(), alloc_get_free());

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
        kdebug(DEBUGSRC_MM, ERR_TRACE, "  -> %08X WH %d:%d", allocs[block][index].addr, block, index);
        return (void *)allocs[block][index].addr;
    }
    
    struct alcent ae = { .valid = 1, .used = 1, .addr = allocs[block][index].addr, .size = sz };

    if(allocs[block][index].addr % align) {
        /* Block isn't perfectly aligned */
        size_t correction = align - (allocs[block][index].addr % align);
        ae.addr = allocs[block][index].addr + correction;
        if(allocs[block][index].size > (sz + correction)) {
            /* Block also extends beyond needed space */
            struct alcent fe = {.valid = 1, .used = 0,
                                .addr = allocs[block][index].addr + (sz + correction),
                                .size = allocs[block][index].size - (sz + correction) };
            add_alloc(&fe);
        }
        allocs[block][index].size = correction;
        kdebug(DEBUGSRC_MM, ERR_TRACE, "  -> %08X PU %d:%d", ae.addr, block, index);
    } else {
        /* Block is aligned, but has extra space at the end */
        allocs[block][index].size -= sz;
        allocs[block][index].addr += sz;
        kdebug(DEBUGSRC_MM, ERR_TRACE, "  -> %08X PA %d:%d", ae.addr, block, index);
    }

    add_alloc(&ae);

    // Let other processes allocate memory
    unlock(&alloc_lock);

    return (void *)ae.addr;
}
EXPORT_FUNC(kamalloc);

void *kmalloc(size_t sz) {
    return kamalloc(sz, MIN_ALIGN);
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
    kdebug(DEBUGSRC_MM, ERR_TRACE, "Freeing %08X", ptr);

    lock(&alloc_lock);

    int i, j = 0;

    // Find the corresponding memory block
    for(; j < ALLOC_BLOCKS; j++) {
        if(!allocs[j]) {
            continue;
        }
        for(i = 0; i < ALLOC_BLOCK; i++) {
            if(allocs[j][i].valid) { // Is it valid?
                if(allocs[j][i].addr == (uint32_t)ptr) { // Is it the correct block?
                    rm_alloc(j, i); // Free it!
                    unlock(&alloc_lock);
                    return;
                }
            }
        }
    }

    kpanic("Attempt to free address not previously allocated: %p", ptr);
}
EXPORT_FUNC(kfree);


void *malloc(size_t sz) {
    void *ret = kmalloc(sz);
    if(!ret) {
        return NULL;
    }
    kproc_t *proc = mtask_get_curr_process();
    if(proc) {
        /* NOTE: Assuming one-to-one mapping. */
        mm_proc_mmap_add(proc, (uintptr_t)ret, (uintptr_t)ret, sz);
    }

    return ret;
}

void free(void *ptr) {
    kproc_t *proc = mtask_get_curr_process();
    if(proc) {
        /* NOTE: Assuming allocation table is using physical addresses. */
        mm_proc_mmap_remove_phys(proc, (uintptr_t)ptr);
    }
    kfree(ptr);
}


size_t alloc_get_used() {
    size_t used = 0;

    for(size_t blk = 0; blk < ALLOC_BLOCKS; blk++) {
        for(size_t ent = 0; ent < ALLOC_BLOCK; ent++) {
            if(allocs[blk] &&
               allocs[blk][ent].valid &&
               allocs[blk][ent].used) {
                used += allocs[blk][ent].size;
            }
        }
    }

    return used;
}

size_t alloc_get_free() {
    size_t free = 0;

    for(size_t blk = 0; blk < ALLOC_BLOCKS; blk++) {
        for(size_t ent = 0; ent < ALLOC_BLOCK; ent++) {
            if(allocs[blk] &&
               allocs[blk][ent].valid &&
               !allocs[blk][ent].used) {
                free += allocs[blk][ent].size;
            }
        }
    }

    return free;
}


/**
 * Initialize allocation functions
 *
 * @param base base of the usable memory
 * @param size amount of usable memory
 */
void init_alloc(uint32_t base, uint32_t size) {
    struct alcent ae = { .valid = 1, .used = 0, .addr = base, .size = size };

    // Add a free allocation block encompasing all the usable memory
    add_alloc(&ae);

    /* Default to all pages r/w and identity-mapped */
    mmu_map(base, base, size, MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL);
}
