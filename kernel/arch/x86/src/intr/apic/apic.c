#include <err/error.h>
#include <err/panic.h>

#include <string.h>

#include <arch/intr/types/intr.h>
#include <arch/intr/apic/apic.h>
#include <arch/intr/apic/ioapic.h>
#include <arch/intr/pic.h>
#include <arch/acpi/acpi.h>
#include <arch/io/cpuid.h>
#include <arch/io/msr.h>

#include <mm/mmu.h>

static struct {
    uint32_t            lapic_count;
    apic_ioapic_regs_t *ioapic;
    uintptr_t           lapic64;

    /* Mapping of legacy PIC IRQs to IOAPIC IRQs */
    uint8_t ioapic_irq_map[16];
} _apic_data;

__unused
static uint32_t _ioapic_read(uint32_t);
__unused
static void _ioapic_write(uint32_t, uint32_t);

__unused
static void _apic_enable(void) {
    uint64_t lapic = msr_read(MSRREG_APICBASE);
    lapic |= MSR_APICBASE_ENABLE;
    msr_write(MSRREG_APICBASE, lapic);
}

void apic_init(void) {
    memset(&_apic_data, 0, sizeof(_apic_data));

    if(!cpuid_avail() ||
       !cpuid_featurecheck_edx(CPUIDFEATURE_EDX_APIC)) {
        return;
    }

    for(uint8_t i = 0; i < 16; i++) {
        /* Assume 1-to-1 unless MADT tells us otherwise */
        _apic_data.ioapic_irq_map[i] = i;
    }

    const acpi_madt_t *madt = (const acpi_madt_t *)acpi_find_sdt("APIC");

    if(!madt) {
        return;
    }

    kerror(ERR_DEBUG, "apic: MADT table found at %08X", madt);
    kerror(ERR_DEBUG, "  flags: %08X", madt->flags);
    kerror(ERR_DEBUG, "  lapic: %08X", madt->lapic_addr);
    kerror(ERR_TRACE, "  ENTRIES:");

    const acpi_madt_entry_t *ent = (const acpi_madt_entry_t *)madt->entries;
    uintptr_t                end = (uintptr_t)madt + madt->head.length;
    while((uintptr_t)ent < end) {
        if(ent->type == ACPI_MADT_ENTRYTYPE_LOCALAPIC) {
            const acpi_madt_entry_lapic_t *lapic = (acpi_madt_entry_lapic_t *)ent->data;
            kerror(ERR_TRACE, "    [%hhu] Proc: %3hhd, ID: %3hhd, Flags: %08X",
                   ent->type, lapic->processor_id, lapic->apic_id, lapic->flags);
            if(lapic->flags & (ACPI_MADT_LAPIC_FLAG_ENABLED |
                               ACPI_MADT_LAPIC_FLAG_ONLINEABLE)) {
                _apic_data.lapic_count++;
            }
        } else if(ent->type == ACPI_MADT_ENTRYTYPE_IOAPIC) {
            const acpi_madt_entry_ioapic_t *ioapic = (acpi_madt_entry_ioapic_t *)ent->data;
            kerror(ERR_TRACE, "    [%hhu] ID: %3hhd, ADDR: %08X, BASE: %08X",
                   ent->type, ioapic->apic_id, ioapic->apic_addr, ioapic->gsi_base);
            /* TODO: Support multiple IOAPICs */
            if(!_apic_data.ioapic) {
                _apic_data.ioapic = (apic_ioapic_regs_t *)ioapic->apic_addr;
            }
        } else if(ent->type == ACPI_MADT_ENTRYTYPE_LAPICADDROVER) {
            const acpi_madt_entry_lapic_addrover_t *lapic = (acpi_madt_entry_lapic_addrover_t *)ent->data;
            kerror(ERR_TRACE, "    [%hhu] ADDR: %08X",
                   ent->type, lapic->lapic_addr);
            /*_apic_data.lapic = (uintptr_t)lapic->lapic_addr;*/
        } else if(ent->type == ACPI_MADT_ENTRYTYPE_IOAPICSRCOVER) {
            const acpi_madt_entry_ioapic_srcover_t *sover = (acpi_madt_entry_ioapic_srcover_t *)ent->data;
            kerror(ERR_DEBUG, "    [%hhu] SRC: (%3hhd:%3hhd), GSI: %3d, FLAGS: %04hX",
                   ent->type, sover->bus_src, sover->irq_src, sover->gsi, sover->flags);
            if ((sover->bus_src == 0) && (sover->irq_src < 16)) {
                /* PIC to IOAPIC mapping */
                _apic_data.ioapic_irq_map[sover->irq_src] = sover->gsi;
            }
        } else {
            kerror(ERR_DEBUG, "    [%hhu]", ent->type);
        }

        ent = (const acpi_madt_entry_t *)((uintptr_t)ent + ent->length);
    }

    kerror(ERR_DEBUG, "  Found %d local APIC(s)", _apic_data.lapic_count);

    _apic_enable();

    /* TODO: Do this per-core, perhaps someplace else */
    mmu_map(0xFEE00000, 0xFEE00000, 0x1000,
            MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL | MMU_FLAG_NOCACHE);
    apic_lapic_handle_t lapic;
    apic_lapic_init(&lapic, 0xFEE00000);

    ioapic_init();
    /* Emulate legacy PIC behaviour for now */
    for (uint8_t i = 0; i < 16; i++) {
        pic_irq_disable(i);
        ioapic_map_irq(_apic_data.ioapic, _apic_data.ioapic_irq_map[i], 32 + i);
    }
}

uintptr_t apic_getaddr(void) {
    uint64_t lapic = msr_read(MSRREG_APICBASE);
    return (uintptr_t)(lapic & (MSR_APICBASE_BASE__MASK << MSR_APICBASE_BASE__POS));
}

apic_ioapic_regs_t *apic_get_ioapic(void) {
    return _apic_data.ioapic;
}

int apic_lapic_init(apic_lapic_handle_t *hand, uintptr_t base) {
    memset(hand, 0, sizeof(apic_lapic_handle_t));

    hand->regs = (apic_lapic_regs_t *)base;

    hand->regs->lvt_error.value = INTR_APICERROR;
    /* Allow all interrupts */
    hand->regs->task_prio.value = 0;
    /* Enable APIC */
    hand->regs->spurious_int.value = 0xFF | (1UL << LAPIC_SPURIOUS_APICEN__POS);

    return 0;
}


static uint32_t _ioapic_read(uint32_t reg) {
    if(!SAFETY_CHECK(_apic_data.ioapic)) {
        kpanic("IOAPIC address NULL!");
    }
    volatile uint32_t *ioapic = (volatile uint32_t *)_apic_data.ioapic;
    ioapic[0] = reg;
    return ioapic[4];
}

static void _ioapic_write(uint32_t reg, uint32_t val) {
    if(!SAFETY_CHECK(_apic_data.ioapic)) {
        kpanic("IOAPIC address NULL!");
    }
    volatile uint32_t *ioapic = (volatile uint32_t *)_apic_data.ioapic;
    ioapic[0] = reg;
    ioapic[4] = val;
}


static int _intctlr_intr_enable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -1;
    }

    apic_lapic_handle_t *hand = (apic_lapic_handle_t *)data;

    /* TODO */
    (void)hand;

    return 0;
}

static int _intctlr_intr_disable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -1;
    }

    apic_lapic_handle_t *hand = (apic_lapic_handle_t *)data;

    /* TODO */
    (void)hand;

    return 0;
}

static int _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data) {
    if (int_n > 255) {
        return -1;
    }

    apic_lapic_handle_t *hand = (apic_lapic_handle_t *)data;

    /* TODO */
    (void)hand;
    (void)callback;
    (void)int_data;

    return 0;
}

int apic_lapic_create_intctlrdev(apic_lapic_handle_t *hand, hal_intctlr_dev_t *intctlrdev) {
    memset(intctlrdev, 0, sizeof(hal_intctlr_dev_t));

    intctlrdev->data = (void *)hand;

    intctlrdev->intr_enable  = _intctlr_intr_enable;
    intctlrdev->intr_disable = _intctlr_intr_disable;
    intctlrdev->intr_attach  = _intctlr_intr_attach;
    
    intctlrdev->cap = HAL_INTCTLRDEV_CAP_INTENDISABLE | HAL_INTCTLRDEV_CAP_INTATTACH;

    return 0;
}
