#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <arch/intr/int.h>

#include <types.h>

/* TODO: Add platform-agnosting interrupt_enable/_disable functions */

/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(interrupt_idx_e n, void *handler);

/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum);

#endif
