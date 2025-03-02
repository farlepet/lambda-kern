#include <types.h>
#include <string.h>

#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>
#include <arch/mm/mmu.h>

/**
 * Kernel entry-point: performs target-specific system initialization.
 */
__noreturn __optimize_none
static void kentry(uint32_t board, uint32_t machine, void *atags) {
    (void)board;
    (void)machine;
    (void)atags;

    arch_init();

    fs_init();

    kmain();
}


/* Temporary MMU table to map higher-half kernel.
 * @todo We should be able to re-use this table later on, rather than replacing
 * it. */
__align(16384)
static uint32_t __init_mmu_table[4096];

/* @note The compiler was setting `low_mmu_table to the virtual address
 * regardless of optimization. This forces the math to actually take place. */
__optimize_none
void *__get_low_addr(void *addr) {
    return addr - 0xC0000000;
}

/* Values defined in linker script */
extern uint8_t __kernel_bss_begin;
extern uint8_t __kernel_bss_end;
static void _data_init(void) {
    void *start = __get_low_addr(&__kernel_bss_begin);
    void *end   = __get_low_addr(&__kernel_bss_end);
    memset(&start, 0, (&end - &start));
}

__noreturn
void kern_premap(uint32_t board, uint32_t machine, void *atags) {
    uint32_t *low_mmu_table = __get_low_addr(__init_mmu_table);

    _data_init();

    /* Clear out page directory */
    for(int i = 0; i < 4096; i++) {
        low_mmu_table[i] = 0;
    }

    /* Temporarially map lower 1 MiB low and high */
    low_mmu_table[0]    = MMU_DESCTYPE_SECTION | (1UL << MMU_DESC_S_B__POS) |
                                                 (1UL << MMU_DESC_S_C__POS);
    /* @todo Do not hard-code higher-half address. */
    low_mmu_table[3072] = MMU_DESCTYPE_SECTION | (1UL << MMU_DESC_S_B__POS) |
                                                 (1UL << MMU_DESC_S_C__POS);

    uint32_t r = 0;
    __mcr(r, p15, 0, c7, c5, 0); /* Invalidate cache - instruction */
    __mcr(r, p15, 0, c7, c6, 1); /* Invalidate cache - data*/
    __mcr(r, p15, 0, c8, c7, 0); /* Invalidate TLB */

    r = 0xFFFFFFFF;              /* Do not check permissions on all 16 domains */
    __mcr(r, p15, 0, c3, c0, 0); /* Domain Access Control Register*/


    r = 0x00000000;              /* Only use TTBR0 */
    __mcr(r, p15, 0, c2, c0, 2); /* Translation Table Base Control Register */

    __mcr(low_mmu_table, p15, 0, c2, c0, 0); /* Set Translation Table 0 Base */
    __mcr(low_mmu_table, p15, 0, c2, c0, 1); /* Set Translation Table 1 Base */

    /* System Control Register
     *  - MMU Enable */
    __mrc(r, p15, 0, c1, c0, 0);
    r |= 0x00000001;
    __mcr(r, p15, 0, c1, c0, 0);

    /* @note Not exactly the best way to do this, but since ARM uses relative-
     * addressing, and since we have both low and high mapped, it should work. */
    asm volatile("add sp, sp, #0xC0000000\n"
                 "add pc, pc, #0xC0000000\n");

    kentry(board, machine, atags);
}
