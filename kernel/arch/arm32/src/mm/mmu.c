#include <errno.h>
#include <string.h>

#include <arch/mm/mmu.h>
#include <arch/registers.h>

#include <err/panic.h>

#include <mm/mmu.h>
#include <mm/alloc.h>

__align(16384)
static uint32_t _mmu_table[4096];  /* Kernel MMU table */
__align(4096)
static uint32_t _page_table0[256]; /* Mapping lower 1 MiB of memory */

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

    r = 0x00000000;              /* Only use TTBR0 */
    __mcr(r, p15, 0, c2, c0, 2); /* Translation Table Base Control Register */

    __mcr(base, p15, 0, c2, c0, 0); /* Set Translation Table 0 Base */
    __mcr(base, p15, 0, c2, c0, 1); /* Set Translation Table 1 Base */

    /* System Control Register
     *  - MMU Enable */
    __mrc(r, p15, 0, c1, c0, 0);
    r |= 0x00000001;
    __mcr(r, p15, 0, c1, c0, 0);
}

int armv7_mmu_init(void) {
    memset(_mmu_table, 0, sizeof(_mmu_table));
    /* Simple map for initial testing: */
    for(uint32_t i = 0; i < 1024; i++) {
        uint32_t addr = i * 1024 * 1024;
        /* @todo Do not hard-code kernel region start address. */
        _mmu_map_section(_mmu_table, addr + 0xC0000000, addr, 0);
        /* Temporary work-around for ident-mapping allocations */
        _mmu_map_section(_mmu_table, addr, addr, (1UL << MMU_DESC_S_XN__POS));
    }

    _page_table0[0] = 0; /* Trap NULL pointers */
    _mmu_table[0] = ((uint32_t)_page_table0 - 0xC0000000) | MMU_DESCTYPE_PAGETABLE;
    for(uint32_t i = 1; i < 256; i++) {
        uint32_t addr = i * 4096;
        _mmu_map_page(_mmu_table, addr, addr, (1UL << MMU_PTENTRY_SMALL_XN__POS));
    }

    _mmu_setup((uintptr_t)_mmu_table - 0xC0000000);

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
    if(virt >= 0xC0000000) {
        /* @note Currently hard-coding kernel-space mapping to be ident. Should
         * be addressed in the future. */
        return 0;
    }
    /* @note Only supporting small pages at the moment */
    uint32_t pflags = 0;
    if(!(flags & MMU_FLAG_EXEC)) {
        pflags |= (1UL << MMU_PTENTRY_SMALL_XN__POS);
    }
    if(!(flags & MMU_FLAG_NOCACHE)) {
        pflags |= (1UL << MMU_PTENTRY_B__POS) |
                  (1UL << MMU_PTENTRY_C__POS);

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

    return -EUNSPEC;
}

int mmu_map_get_table(mmu_table_t *table, uintptr_t virt, uintptr_t *phys) {
    unsigned tidx = (virt >> 20);

    if((table->mmu_table[tidx] & MMU_DESCTYPE__MASK) == 0) {
        return 0;
    } else if ((table->mmu_table[tidx] & MMU_DESCTYPE__MASK) == MMU_DESCTYPE_PAGETABLE) {
        uint32_t pidx = (virt >> 12) & 0xFF;
        uint32_t *ptable = (uint32_t *)(table->mmu_table[tidx] & (MMU_DESC_PT_ADDR__MASK << MMU_DESC_PT_ADDR__POS));

        if((ptable[pidx] & MMU_PTENTRYTYPE__MASK) == 0) {
            return 0;
        } else if((ptable[pidx] & MMU_PTENTRYTYPE__MASK) == MMU_PTENTRYTYPE_LARGE) {
            if(phys) {
                *phys = ptable[pidx] & (MMU_PTENTRY_LARGE_BASE__MASK << MMU_PTENTRY_LARGE_BASE__POS);
            }

            if(!(ptable[pidx] & (1UL << MMU_PTENTRY_LARGE_XN__POS))) {
                return (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC);
            } else {
                return (MMU_FLAG_READ | MMU_FLAG_WRITE);
            }
        } else {
            if(phys) {
                *phys = ptable[pidx] & (MMU_PTENTRY_SMALL_BASE__MASK << MMU_PTENTRY_SMALL_BASE__POS);
            }

            if(!(ptable[pidx] & (1UL << MMU_PTENTRY_SMALL_XN__POS))) {
                return (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC);
            } else {
                return (MMU_FLAG_READ | MMU_FLAG_WRITE);
            }
        }
    } else {
        if(phys) {
            *phys = (table->mmu_table[tidx] & (MMU_DESC_S_BASE__MASK << MMU_DESC_S_BASE__POS));
        }

        if(!(table->mmu_table[tidx] & (1UL << MMU_DESC_S_XN__POS))) {
            return (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC);
        } else {
            return (MMU_FLAG_READ | MMU_FLAG_WRITE);
        }
    }
}

mmu_table_t *mmu_clone_table(mmu_table_t *src) {
    return src;
}

