#include <types.h>
#include "paging.h"

#define N_ALLOCS 1024

struct alcent //!< Describes a block of memory
{
	u8  valid; //!< Is this a valid block? (If not, it can be replaced)
	u8  used;  //!< Is this describing used memory?
	u32 addr;  //!< The address of the block of memory
	u32 size;  //!< Size of the block of memory
};

/**
 * Allocates a block of memory
 *
 * @param sz size of the required memory block
 * @returns pointer to memory block
 */
void *kmalloc(u32 sz);

/**
 * Free an allocated memory block
 *
 * @param pointer to previously allocated memory block
 */
void kfree(void *ptr);

/**
 * Initialize allocation functions.
 *
 * @param base location of usable memory
 * @param size size of usable memory
 */
void init_alloc(u32 base, u32 size);
