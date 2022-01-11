#include <arch/mm/mmu.h>
#include <arch/registers.h>

#include <err/panic.h>

__align(4096)
static uint32_t _mmu_table[4096];



static void _mmu_map_section(uint32_t *table, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t idx = (virt >> 20);
    table[idx] = (phys & (MMU_DESC_S_BASE__MASK << MMU_DESC_S_BASE__POS)) |
                 flags                                                    |
                 MMU_DESCTYPE_SECTION;
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
    /* Simple identity map for initial testing: */
    for(uint32_t i = 0; i < 4096; i++) {
        uint32_t addr = i * 1024 * 1024;
        _mmu_map_section(_mmu_table, addr, addr, 0);
    }
    _mmu_map_section(_mmu_table, 0, 0, (1UL << MMU_DESC_S_B__POS) |
                                        (1UL << MMU_DESC_S_C__POS));

    _mmu_setup((uintptr_t)_mmu_table);

    return 0;
}