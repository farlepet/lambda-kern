#include "arch/intr/apic/types/apic.h"
#include <arch/intr/apic/apic.h>
#include <arch/intr/apic/ioapic.h>

#include <err/error.h>
#include <mm/mmu.h>

static uint32_t _ioapic_read(apic_ioapic_regs_t *ioapic, uint8_t addr) {
    ioapic->ioregsel.value = addr;
    return ioapic->iowin.value;
}

static void _ioapic_write(apic_ioapic_regs_t *ioapic, uint8_t addr, uint32_t value) {
    ioapic->ioregsel.value = addr;
    ioapic->iowin.value = value;
}

int ioapic_init(void) {
    apic_ioapic_regs_t *ioapic = apic_get_ioapic();
    if(!ioapic) {
        return -1;
    }
    mmu_map((uintptr_t)ioapic, (uint32_t)ioapic, 0x1000,
            MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL | MMU_FLAG_NOCACHE);

    kerror(ERR_DEBUG, "IOAPIC found:");
    kerror(ERR_DEBUG, "       ID: %u", (_ioapic_read(ioapic, IOAPIC_ADDR_ID) >> 24) & 0x0F);
    kerror(ERR_DEBUG, "  Version: %u", _ioapic_read(ioapic, IOAPIC_ADDR_VER) & 0xFF);
    kerror(ERR_DEBUG, "   Arb ID: %u", (_ioapic_read(ioapic, IOAPIC_ADDR_ARBID) >> 24) & 0x0F);

    return 0;
}

int ioapic_map_irq(apic_ioapic_regs_t *ioapic, uint8_t irq, uint8_t vector) {
    uint32_t ver = _ioapic_read(ioapic, IOAPIC_ADDR_VER);
    if(irq > ((ver >> IOAPIC_VER_VER__POS) & IOAPIC_VER_VER__MSK)) {
        return -1;
    }

    uint8_t addr = IOAPIC_ADDR_REDTBL_BASE + (irq * 2);

    uint64_t entry =  (uint64_t)_ioapic_read(ioapic, addr) |
                     ((uint64_t)_ioapic_read(ioapic, addr + 1) << 32);

    entry = (entry & ~(IOAPIC_REDTBL_INTVEC__MSK << IOAPIC_REDTBL_INTVEC__POS)) |
            (vector << IOAPIC_REDTBL_INTVEC__POS);
    /* TODO: Create separate functions for masking/unmasking */
    entry &= ~(1ULL << IOAPIC_REDTBL_INTMSK__POS);

    kerror(ERR_DEBUG, "IOAPIC[%hhu] setting to %016llx", irq, entry);

    _ioapic_write(ioapic, addr, (uint32_t)entry);
    _ioapic_write(ioapic, addr, (uint32_t)(entry >> 32));

    return 0;
}
