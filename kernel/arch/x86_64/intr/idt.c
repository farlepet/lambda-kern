#include <types.h>
#include <ioport.h>
#include "idt.h"

extern void load_idt(u64 *, u32); //!< Use `lidt` to set the IDT pointer.
extern void dummy_int(); //!< Dummy interrupt se all IRQ's can function, even if not setup.

struct idt_entry_64 IDT[256]; //!< Interrupt Descriptor Table. Table of all interrupt handlers and settings.

/**
 * \brief Remap the IRQ's to fire interrupts after the offsets.
 * Remaps the PIC so when an IRQ fires, it adds `offx` to the IRQ number.
 * Which offset is used depends on wether the IRQ came from the master or
 * the slave PIC.
 * @param off1 the offset for the master PIC
 * @param off2 the offset for the slave PIC
 */
static void remap_pic(int off1, int off2)
{
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, off1);
	outb(0xA1, off2);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

/**
 * \brief Loads the IDT pointer.
 * Loads the IDT pointer, then remaps the PIC.
 * @see load_idt
 * @see remap_pic
 */
static void reload_idt()
{
	load_idt((u64 *)&IDT[0], sizeof(IDT)-1);
	remap_pic(0x20, 0x28);
}

/**
 * \brief Initializes the IDT
 * Initializes the IDT by first setting every IDT entry to use the dummy
 * interrupt then loads the IDT pointer and remaps the PIC.
 * @see reload_idt
 */
void idt_init()
{
	int i = 0;
	for(; i < 256; i++)
		IDT_ENTRY(IDT[i], (u64)&dummy_int, 0x08, 0x8E);
	
	reload_idt();
}

/**
 * \brief Set an entry in the IDT.
 * Sets the information in an IDT entry.
 * @param intr the interrupt number
 * @param sel the GDT selector
 * @param flags the entrys flags (@see IDT_ATTR)
 * @param func the interrupt handler function
 */
void set_idt(int intr, int sel, int flags, void *func)
{
	IDT_ENTRY(IDT[intr], (u64)func, sel, flags);
}