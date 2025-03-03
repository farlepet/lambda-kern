#ifndef ARCH_INTR_APIC_H
#define ARCH_INTR_APIC_H

#include <types.h>

#include <hal/intr/int_ctlr.h>

#include <arch/intr/apic/types/apic.h>

void apic_init(void);

uintptr_t apic_getaddr(void);

/**
 * @brief Get address of the IOAPIC, if found, else returns 0.
 */
apic_ioapic_regs_t *apic_get_ioapic(void);

int apic_lapic_init(apic_lapic_handle_t *hand, uintptr_t base);

int apic_lapic_create_intctlrdev(apic_lapic_handle_t *hand, hal_intctlr_dev_t *intctlrdev);

#define MSR_APICBASE_BASE__POS  (                12)
#define MSR_APICBASE_BASE__MASK (0xFFFFFFFFFFFFFULL) /* Actual number of valid bits depends on hardware */
#define MSR_APICBASE_ENABLE     (1UL          << 11)
#define MSR_APICBASE_X2APIC     (1UL          << 10)
#define MSR_APICBASE_BSP        (1UL          <<  8)

#endif
