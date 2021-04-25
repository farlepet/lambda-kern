#ifndef MM_MMAP_H
#define MM_MMAP_H

#include <stdint.h>
#include <sys/types.h>

/**
 * \brief mmap
 * 
 * @param addr Desired address, 0 if specific address not required
 * @param len Length of memory desired
 * @param prot Requested permissions
 * @param flags Map type
 * @param filedes File descriptor, or -1
 * @param off Offset into file
 * 
 * @return Memory-mapped region, NULL on error
 */
void *mmap(void *addr, size_t len, int prot, int flags, int filedes, off_t off);

/**
 * \brief munmap
 * 
 * @param addr Start address of mapped region
 * @param len Length of mapped region
 */
int munmap(void *addr, size_t len);

#endif