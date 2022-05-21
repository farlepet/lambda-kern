#ifndef MM_MMU_H
#define MM_MMU_H

#include <stdint.h>

#include <arch/types/mmu.h>

#define MMU_FLAG_READ   (0x00000001) /** Mapped memory can be read from */
#define MMU_FLAG_WRITE  (0x00000002) /** Mapped memory can be written to */
#define MMU_FLAG_EXEC   (0x00000004) /** Mapped memory can be executed */
#define MMU_FLAG_KERNEL (0x00000010) /** Mapped memory belongs to the kernel, and cannot be accessed by user processes */

/**
 * @brief Get page size currently in use on the system
 *
 * If supports multiple page sizes concurrently, the smallest is returned.
 *
 * @note This function is implemented per-architecture
 *
 * @return size_t 0 on failure, else page size
 */
size_t mmu_get_pagesize(void);

/**
 * @brief Get MMU table currently in use
 *
 * @note This function is implemented per-architecture
 *
 * @return mmu_table_t* NULL on failure, else pointer to MMU table
 */
mmu_table_t *mmu_get_current_table(void);

/**
 * @brief Get MMU table used by the kernel
 *
 * @note This function is implemented per-architecture
 *
 * @return mmu_table_t* NULL on failure, else pointer to MMU table
 */
mmu_table_t *mmu_get_kernel_table(void);

/**
 * @brief Sets current MMU table
 *
 * @note This function is implemented per-architecture
 *
 * @param table Pointer to MMU table
 *
 * @return int 0 on success, -1 on failure
 */
int mmu_set_current_table(mmu_table_t *table);

/**
 * @brief Map virtual address range to physical address range, in a given MMU table
 *
 * @note This function is implemented per-architecture
 *
 * @param table MMU table to map within
 * @param virt Start of virtual address range, rounded down to nearest page boundry
 * @param phys Start of physucal address range, rounded down to nearest page boundry
 * @param size Size of address range, rounded up to nearest multiple of page size
 * @param flags Flags affecting mapping, see MMU_FLAG_*
 *
 * @return int 0 on success, else -1
 */
int mmu_map_table(mmu_table_t *table, uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags);

/**
 * @brief Map virtual address range to physical address range, in the current MMU table
 *
 * @see mmu_map_table
 *
 * @param virt Start of virtual address range, rounded down to nearest page boundry
 * @param phys Start of physucal address range, rounded down to nearest page boundry
 * @param size Size of address range, rounded up to nearest multiple of page size
 * @param flags Flags affecting mapping, see MMU_FLAG_*
 *
 * @return int 0 on success, else -1
 */
int mmu_map(uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags);

/**
 * @brief Unmap a virtuall address range, in a given MMU table
 *
 * @note This function is implemented per-architecture
 *
 * @param table MMU table to map within
 * @param virt Start of virtual address range, rounded down to nearest page boundry
 * @param size Size of address range, rounded up to nearest multiple of page size
 *
 * @return int 0 on success, else -1
 */
int mmu_unmap_table(mmu_table_t *table, uintptr_t virt, size_t size);

/**
 * @brief Unmap a virtual address range, in the current MMU table
 *
 * @param virt Start of virtual address range, rounded down to nearest page boundry
 * @param size Size of address range, rounded up to nearest multiple of page size
 *
 * @return int 0 on success, else -1
 */
int mmu_unmap(uintptr_t virt, size_t size);

/**
 * @brief Get the physical address mapped to the given virtual address, and its
 * flags, in the given MMU table
 *
 * @note This function is implemented per-architecture
 *
 * @param table MMU table to map within
 * @param virt Virtual address, rounded down to nearest page boundry
 * @param phys[out] Physical address pointed to by virutal address
 *
 * @return int -1 on failure, else flags (see MMU_FLAG_*)
 */
int mmu_map_get_table(mmu_table_t *table, uintptr_t virt, uintptr_t *phys);

/**
 * @brief Get the physical address mapped to the given virtual address, and its
 * flags, in the current MMU table
 *
 * @param virt Virtual address, rounded down to nearest page boundry
 * @param phys[out] Physical address pointed to by virutal address
 *
 * @return int -1 on failure, else flags (see MMU_FLAG_*)
 */
int mmu_map_get(uintptr_t virt, uintptr_t *phys);

/**
 * @brief Clones MMU table
 *
 * @note This function is implemented per-architecture
 *
 * @todo Allow for partial clones (e.g. kernel only)
 *
 * @param src Source MMU table
 *
 * @return mmu_table_t* NULL on error, else newly cloned MMU table
 */
mmu_table_t *mmu_clone_table(mmu_table_t *src);

#endif
