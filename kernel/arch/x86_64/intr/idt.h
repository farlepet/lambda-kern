/** \file idt.h
 *  \brief Contains definitions for the IDT.
 *
 *  Contains functions and defenitions that allow for the setup and usage
 *  of the IDT and interrupts in general.
 */

#include <types.h>

#ifndef IDT_H
#define IDT_H

/**
 * \brief Allows for easy creation of an IDT entry.
 * Helps easily create an IDT entry.
 */
struct  idt_entry_64 {
	u16 baseLow;
	u16 selector;
	u8  reservedIst;
	u8  flags;
	u16 baseMid;
	u32 baseHigh;
	u32 reserved;
} __packed;

#define IDT_ENTRY(ent, base, sel, attr)              \
	do                                                \
	{                                                  \
		ent.baseLow  = base & 0xFFFF;                   \
		ent.baseMid  = (base & 0xFFFF0000) >> 16;        \
		ent.baseHigh = (base & 0xFFFFFFFF00000000) >> 32; \
		ent.flags    = attr;                               \
		ent.selector = sel;                                 \
		ent.reserved = 0;                                    \
		ent.reservedIst = 0;                                  \
	} while(0);

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
	((dpl      & 0x03) << 5) | \
	((storeseg & 0x01) << 4) | \
	((type     & 0x0F) << 0))


/**
 * \brief Initializes the IDT
 * Initializes the IDT by first setting every IDT entry to use the dummy
 * interrupt then loads the IDT pointer and remaps the PIC.
 * @see reload_idt
 */
void idt_init();

/**
 * \brief Set an entry in the IDT.
 * Sets the information in an IDT entry.
 * @param intr the interrupt number
 * @param sel the GDT selector
 * @param flags the entrys flags
 * @param func the interrupt handler function
 * @see IDT_ATTR
 */
void set_idt(int, int, int, void *);


int disable_irq(u8 irq);

int enable_irq(u8 irq);

#endif