#ifndef ARCH_INTR_APIC_IOAPIC_H
#define ARCH_INTR_APIC_IOAPIC_H

#include <arch/intr/apic/types/apic.h>

int ioapic_init(void);

int ioapic_map_irq(apic_ioapic_regs_t *ioapic, uint8_t irq, uint8_t vector);

#endif

