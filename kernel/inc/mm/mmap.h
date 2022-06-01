#ifndef MM_MMAP_H
#define MM_MMAP_H

#include <stdint.h>
#include <sys/types.h>

#define SYSCALL_MMAPFLAG_TYPE      (0x0F)
#define SYSCALL_MMAPFLAG_PRIVATE   (1UL) /** Private mapping for this process only */
#define SYSCALL_MMAPFLAG_SHARED    (2UL) /** Mapping is shared with other processes */

#define SYSCALL_MMAPFLAG_FIXED     (1UL << 4) /** Use addr as physical address */
#define SYSCALL_MMAPFLAG_ANONYMOUS (1UL << 5) /** Memory is not bound to a file */

#define SYSCALL_MMAPPROT_NONE      (0)        /** No permissions */
#define SYSCALL_MMAPPROT_READ      (1UL << 0) /** Memory can be read */
#define SYSCALL_MMAPPROT_WRITE     (1UL << 1) /** Memory can be written */
#define SYSCALL_MMAPPROT_EXEC      (1UL << 2) /** Memory can be executed */

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
 * @return Virtual address of region, NULL on error
 */
uintptr_t mmap(uintptr_t addr, size_t len, uint32_t prot, uint32_t flags, int filedes, off_t off);

/**
 * \brief munmap
 * 
 * @param addr Start address of mapped region
 * @param len Length of mapped region
 */
int munmap(uintptr_t addr, size_t len);

#endif