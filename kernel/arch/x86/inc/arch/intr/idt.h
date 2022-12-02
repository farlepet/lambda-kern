/** \file idt.h
 *  \brief Contains definitions for the IDT.
 *
 *  Contains functions and defenitions that allow for the setup and usage
 *  of the IDT and interrupts in general.
 */

#ifndef ARCH_INTR_IDT_H
#define ARCH_INTR_IDT_H

#include <stdint.h>

#include <arch/intr/types/idt.h>

/**
 * \brief Initializes the IDT
 * Initializes the IDT by first setting every IDT entry to use the dummy
 * interrupt then loads the IDT pointer and remaps the PIC.
 * @see reload_idt
 */
void idt_init(void);

/**
 * @brief Set an entry in the IDT.
 *
 * @param intr the interrupt number
 * @param sel the GDT selector
 * @param flags the entrys flags
 * @param func the interrupt handler function
 * @see IDT_ATTR
 */
void idt_set_entry(uint8_t intr, uint16_t sel, uint8_t flags, void *func);

/**
 * @brief Add callback for interrupt
 *
 * @param int_n Interrupt number
 * @param hdlr Interrupt handler handle
 *
 * @return 0 on success, else < 0
 */
int idt_add_callback(uint8_t int_n, intr_handler_hand_t *hdlr);

#endif

