#include <string.h>

#include <arch/mm/mmu.h>
#include <arch/registers.h>

#include <err/panic.h>

#include <mm/mmu.h>
#include <mm/alloc.h>

__align(4096)
static uint32_t _mmu_table[4096];  /* Kernel MMU table */

/** Map 1 MiB section */
static void _mmu_map_section(uint32_t *table, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t idx = (virt >> 20);
    table[idx] = (phys & (MMU_DESC_S_BASE__MASK << MMU_DESC_S_BASE__POS)) |
                 flags                                                    |
                 MMU_DESCTYPE_SECTION;
}

/** Map 4 KiB page */
static void _mmu_map_page(uint32_t *table, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t tidx = (virt >> 20);
    uint32_t pidx = (virt >> 12) & 0xFF;

    uint32_t *ptable;

    if((table[tidx] & MMU_DESCTYPE__MASK) != MMU_DESCTYPE_PAGETABLE) {
        /* @note Any other type other than PAGETABLE or INVALID should be handled
         * and translated by the caller */
        ptable = kamalloc(256 * sizeof(uint32_t), 4096);
        memset(ptable, 0, 256 * sizeof(uint32_t));
        table[tidx] = (uint32_t)ptable | MMU_DESCTYPE_PAGETABLE;
    } else {
        ptable = (uint32_t *)(table[tidx] & (MMU_DESC_PT_ADDR__MASK << MMU_DESC_PT_ADDR__POS));
    }

    /* @note Only supporting small pages (4 KiB) for now */
    ptable[pidx] = (phys & (MMU_PTENTRY_SMALL_BASE__MASK << MMU_PTENTRY_SMALL_BASE__POS)) |
                   flags                                                                  |
                   MMU_PTENTRYTYPE_SMALL;
}

static void _mmu_setup(uintptr_t base) {
    if(base & 0xFFF) {
        kpanic("_mmu_setup: Base does not have proper alignment: %08X", base);
    }

    uint32_t r = 0;
    __mcr(r, p15, 0, c7, c5, 0); /* Invalidate cache - instruction */
    __mcr(r, p15, 0, c7, c6, 1); /* Invalidate cache - data*/
    __mcr(r, p15, 0, c8, c7, 0); /* Invalidate TLB */

    r = 0xFFFFFFFF;              /* Do not check permissions on all 16 domains */
    __mcr(r, p15, 0, c3, c0, 0); /* Domain Access Control Register*/

    r = 0x00000002;              /* N = 2 -> 4 KiB alignment */
    __mcr(r, p15, 0, c2, c0, 2); /* Translation Table Base Control Register */

    __mcr(base, p15, 0, c2, c0, 0); /* Set Translation Table 0 Base */
    __mcr(base, p15, 0, c2, c0, 1); /* Set Translation Table 1 Base */

    /* System Control Register
     *  - MMU Enable */
    __mrc(r, p15, 0, c1, c0, 0);
    r |= 0x00800001;
    __mcr(r, p15, 0, c1, c0, 0);
}

int armv7_mmu_init(void) {
    memset(_mmu_table, 0, sizeof(_mmu_table));
    /* Simple identity map for initial testing: */
    /* For now, kernel memory is 0x00000000-0x20000000. This should be moved to
     * the upper half of memory later */
    for(uint32_t i = 0; i < 512; i++) {
        uint32_t addr = i * 1024 * 1024;
        _mmu_map_section(_mmu_table, addr, addr, 0);
    }
    _mmu_map_section(_mmu_table, 0, 0, (1UL << MMU_DESC_S_B__POS) |
                                       (1UL << MMU_DESC_S_C__POS));

    _mmu_setup((uintptr_t)_mmu_table);

    return 0;
}



/* TODO: This is just initial patial support */

size_t mmu_get_pagesize(void) {
    return 1024 * 1024;
}

mmu_table_t *mmu_get_current_table(void) {
    mmu_table_t *table;
    __mrc(table, p15, 0, c2, c0, 0); /* Set Translation Table 0 Base */
    return table;
}

mmu_table_t *mmu_get_kernel_table(void) {
    return (mmu_table_t *)_mmu_table;
}

int mmu_set_current_table(mmu_table_t *table) {
    __mcr(table, p15, 0, c2, c0, 0); /* Set Translation Table 0 Base */
    __mcr(table, p15, 0, c2, c0, 1); /* Set Translation Table 1 Base */

    return 0;
}

int mmu_map_table(mmu_table_t *table, uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags) {
    if(virt < 0x20000000) {
        /* @note Currently hard-coding kernel-space mapping to be ident. Should
         * be addressed in the future. */
        return 0;
    }
    /* @note Only supporting small pages at the moment */
    uint32_t pflags = (1UL << MMU_PTENTRY_B__POS) |
                      (1UL << MMU_PTENTRY_C__POS);
    if(!(flags & MMU_FLAG_EXEC)) {
        pflags |= (1UL << MMU_PTENTRY_SMALL_XN__POS);
    }

    virt &= 0xFFFFF000;
    phys &= 0xFFFFF000;
    for(uint32_t i = 0; i < size; i += 4096) {
        _mmu_map_page((uint32_t *)table, virt, phys, pflags);
        virt += 0x1000;
        phys += 0x1000;
    }

    return 0;
}

int mmu_unmap_table(mmu_table_t *table, uintptr_t virt, size_t size) {
    /* TODO */
    (void)table;
    (void)virt;
    (void)size;

    return -1;
}

int mmu_map_get_table(mmu_table_t *table, uintptr_t virt, uintptr_t *phys) {
    /* TODO: Support non-identity-mapped data, and different tables */
    (void)table;

    *phys = virt & 0xFFF00000;

    return (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC);
}

mmu_table_t *mmu_clone_table(mmu_table_t *src) {
    return src;
}

