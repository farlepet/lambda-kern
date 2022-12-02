#ifndef ARCH_INTR_PIC_H
#define ARCH_INTR_PIC_H

#include <stdint.h>

#include <arch/intr/types/pic.h>

/**
 * @brief Disable an IRQ line
 * 
 * @param irq the IRQ to be disabled
 * @return returns 0 if success
 */
int pic_irq_disable(uint8_t irq);

/**
 * @brief Enable an IRQ line
 * 
 * @param irq the IRQ to be enabled
 * @return returns 0 if success
 */
int pic_irq_enable(uint8_t irq);

/**
 * @brief Remaps the PIC so when an IRQ fires, it adds `offx` to the IRQ number.
 *
 * @param master the offset for the master PIC
 * @param slave the offset for the slave PIC
 */
void pic_remap(uint8_t master, uint8_t slave);

#endif

