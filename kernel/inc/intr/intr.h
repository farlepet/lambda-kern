#ifndef INTR_INTR_H
#define INTR_INTR_H

#include <stdint.h>

#include <intr/types/intr.h>

/* TODO: Add platform-agnosting interrupt_enable/_disable functions */

/**
 * @brief Attaches an interrupt handler to an interrupt.
 *
 * @param n Interrupt ID
 * @param hdlr Interrupt handler handle
 */
void interrupt_attach(interrupt_idx_e n, intr_handler_hand_t *hdlr);

/**
 * @brief Initializes the system timer.
 *
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum);

#endif
