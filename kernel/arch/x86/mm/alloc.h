#include <types.h>
#include "paging.h"

#define ALLOC_BLOCK  1024
#define ALLOC_BLOCKS 512

struct alcent //!< Describes a block of memory
{
	uint8_t  valid; //!< Is this a valid block? (If not, it can be replaced)
	uint8_t  used;  //!< Is this describing used memory?
	uint32_t addr;  //!< The address of the block of memory
	uint32_t size;  //!< Size of the block of memory
};

/**
 * Allocates a block of memory
 *
 * @param sz size of the required memory block
 * @returns pointer to memory block
 */
void *kmalloc(uint32_t sz);

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
void init_alloc(uint32_t base, uint32_t size);
