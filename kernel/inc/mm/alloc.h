#ifndef MM_ALLOC_H
#define MM_ALLOC_H

#include <types.h>

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
 * \brief Allocates a block of memory. Adds accounting to kmalloc.
 * 
 * @param sz Size of requested memory block
 * @return Pointer to memory block on success, else NULL
 */
void *malloc(size_t sz);

/**
 * \brief Frees a previously allocated block of memory. Adds accounting to kfree,
 * 
 * @param ptr Pointer to previously-allocated memory block.
 */
void free(void *ptr);

/**
 * Allocates a block of memory
 *
 * @param sz size of the required memory block
 * @returns pointer to memory block on success, else NULL
 */
void *kmalloc(uint32_t sz);

/**
 * Allocates a block of memory with requested alignment
 *
 * @param sz    size of the required memory block
 * @param align desired alignment
 * @returns pointer to memory block on success, else NULL
 */
void *kamalloc(uint32_t sz, uint32_t align);

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

#endif
