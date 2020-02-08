/** \file idt.h
 *  \brief Contains definitions for the IDT.
 *
 *  Contains functions and defenitions that allow for the setup and usage
 *  of the IDT and interrupts in general.
 */

#ifndef IDT_H
#define IDT_H

/**
 * \brief Allows for easy creation of an IDT entry.
 * Helps easily create an IDT entry.
 * @param off offset of the handler
 * @param sel the GDT selector
 * @param attr the IDT entry's attribute byte
 */
#define IDT_ENTRY(off, sel, attr)    \
	((((uint64_t)off  & 0xFFFF0000ULL) << 32) | \
	 (((uint64_t)attr & 0x000000FFULL) << 40) |  \
	 (((uint64_t)sel  & 0x0000FFFFULL) << 16) |   \
	 (((uint64_t)off  & 0x0000FFFFULL) << 0))

/**
 * \brief Different IDT entry types.
 * Usable types for the attribute flag `type` in the IDT
 */
enum idt_int_type
{
	task32 = 0x05, //!< 32-bit task gate
	int16  = 0x06, //!< 16-bit interrupt gate
	trap16 = 0x07, //!< 16-bit trap gate
	int32  = 0x0E, //!< 32-bit interrupt gate
	trap32 = 0x0F  //!< 32-bit trap gate
};

/**
 * \brief Allows for easy IDT attribute creation.
 * Helps easily create an attribute byte for an entry in the IDT.
 * @param present is the entry present?
 * @param dpl privilege level
 * @param storeseg the GDT segment the handler is located in
 * @param type the type of interrupt this is
 * @see idt_int_type
 */
#define IDT_ATTR(present, dpl, storeseg, type) \
	(((present & 0x01) << 7) | \
	((dpl      & 0x03) << 5) |  \
	((storeseg & 0x01) << 4) |   \
	((type     & 0x0F) << 0))


/**
 * \brief Initializes the IDT
 * Initializes the IDT by first setting every IDT entry to use the dummy
 * interrupt then loads the IDT pointer and remaps the PIC.
 * @see reload_idt
 */
void idt_init(void);

/**
 * \brief Set an entry in the IDT.
 * Sets the information in an IDT entry.
 * @param intr the interrupt number
 * @param sel the GDT selector
 * @param flags the entrys flags
 * @param func the interrupt handler function
 * @see IDT_ATTR
 */
void set_idt(uint8_t intr, int sel, int flags, void *func);

/**
 * Disable an IRQ line
 * 
 * @param irq the IRQ to be disabled
 * @return returns 0 if success
 */
int disable_irq(uint8_t irq);

/**
 * Enable an IRQ line
 * 
 * @param irq the IRQ to be enabled
 * @return returns 0 if success
 */
int enable_irq(uint8_t irq);

#endif