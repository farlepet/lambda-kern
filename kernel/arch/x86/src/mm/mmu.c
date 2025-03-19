#include <errno.h>
#include <string.h>

#include <arch/mm/paging.h>

#include <err/error.h>
#include <mm/alloc.h>
#include <mm/mmu.h>

/* TODO: Possibly move relevant portions of paging.c into this file */

size_t mmu_get_pagesize(void) {
    return PAGE_SZ;
}

mmu_table_t *mmu_get_current_table(void) {
    /* TODO: This is inefficient (nested function call) */
    return (mmu_table_t *)get_pagedir();
}

mmu_table_t *mmu_get_kernel_table(void) {
    return (mmu_table_t *)kernel_cr3;
}

int mmu_set_current_table(mmu_table_t *table) {
    set_pagedir((uint32_t *)table);

    return 0;
}

int mmu_map_table(mmu_table_t *table, uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags) {
    kdebug(DEBUGSRC_MM, ERR_TRACE, "Mapping %p[%u] to %p (%02X) in %p", virt, size, phys, flags, table);

    uint32_t page_flags = PAGE_TABLE_FLAG_PRESENT;

    /* TODO: Caching and global flags */

    if(flags & MMU_FLAG_WRITE)     { page_flags |= PAGE_TABLE_FLAG_WRITABLE; }
    if(!(flags & MMU_FLAG_KERNEL)) { page_flags |= PAGE_TABLE_FLAG_USER; }

    /* NOTE:
     *   All valid memory mapped regions are readable (given permissions)
     *   EXEC is handled within the GDT, and is currently static */

    /* Align to page boundry. Assuming PAGE_SZ 4096 */
    virt &= 0xFFFFF000;
    phys &= 0xFFFFF000;

    for(uintptr_t off = 0; off < size; off += PAGE_SZ) {
        pgdir_map_page((uint32_t *)table, (void *)(phys + off), (void *)(virt + off), page_flags);
    }

    return 0;
}

int mmu_unmap_table(mmu_table_t *table, uintptr_t virt, size_t size) {
    for(uintptr_t off = 0; off < size; off += PAGE_SZ) {
        uint32_t pdindex = (uint32_t)(virt + off) >> 22;
        uint32_t ptindex = (uint32_t)(virt + off) >> 12 & 0x03FF;

        if(table->pagedir_ents[pdindex] & 0x01) {
            uint32_t *pagetable = (uint32_t *)(table->pagedir_ents[pdindex] & 0xFFFFF000);
            pagetable[ptindex] = 0;
        }
    }

    return 0;
}

int mmu_map_get_table(mmu_table_t *table, uintptr_t virt, uintptr_t *phys) {
    uint32_t entry = pgdir_get_page_entry((uint32_t *)table, (void *)virt);

    if(!(entry & PAGE_TABLE_FLAG_PRESENT)) {
        return -EUNSPEC;
    }

    uint32_t flags = MMU_FLAG_READ;
    if(entry & PAGE_TABLE_FLAG_WRITABLE) { flags |= MMU_FLAG_WRITE; }
    if(!(entry & PAGE_TABLE_FLAG_USER))  { flags |= MMU_FLAG_KERNEL; }

    if(phys) {
        *phys = (entry & 0xFFFFF000);
    }

    return (int)flags;
}

mmu_table_t *mmu_clone_table(mmu_table_t *src) {
    size_t n_tables = 0;
    for(size_t i = 0; i < PGDIR_ENTRIES; i++) {
        if(src->pagedir_ents[i] & PAGE_DIRECTORY_FLAG_PRESENT) {
            n_tables++;
        }
    }

    size_t dirsz = sizeof(mmu_table_t) + (0x1000 * n_tables);
    mmu_table_t *new = (mmu_table_t *)kamalloc(dirsz, PAGE_SZ);
    mmu_map((uintptr_t)new, (uintptr_t)new, dirsz, (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL));

    memcpy(new, src, sizeof(mmu_table_t));

    size_t off = 0x1000;
    for(size_t i = 0; i < PGDIR_ENTRIES; i++) {
        if(new->pagedir_ents[i] & PAGE_DIRECTORY_FLAG_PRESENT) {
            new->pagedir_ents[i] = (new->pagedir_ents[i] & 0x00000FFF) |
                                   ((uint32_t)new + off);
            memcpy((void *)(new->pagedir_ents[i] & 0xFFFFF000),
                   (void *)(src->pagedir_ents[i] & 0xFFFFF000), 0x1000);
            off += 0x1000;
        }
    }

    return new;
}
