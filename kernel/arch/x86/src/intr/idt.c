#include <lambda/export.h>
#include <arch/io/ioport.h>
#include <arch/intr/idt.h>

#include <types.h>
#include <err/error.h>

extern void load_idt(uint64_t *, uint32_t); //!< Use `lidt` to set the IDT pointer.
extern void dummy_int(void); //!< Dummy interrupt se all IRQ's can function, even if not setup.

static uint64_t IDT[256]; //!< Interrupt Descriptor Table. Table of all interrupt handlers and settings.

/**
 * Remaps the PIC so when an IRQ fires, it adds `offx` to the IRQ number.
 * Which offset is used depends on wether the IRQ came from the master or
 * the slave PIC.
 *
 * @param off1 the offset for the master PIC
 * @param off2 the offset for the slave PIC
 */
static void remap_pic(uint8_t off1, uint8_t off2)
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
 * Loads the IDT pointer, then remaps the PIC.
 *
 * @see load_idt
 * @see remap_pic
 */
static void _reload_idt(void)
{
    kerror(ERR_INFO, "      -> Loading IDT");
    load_idt(IDT, sizeof(IDT)-1);
    kerror(ERR_INFO, "      -> Remapping IRQ's");
    remap_pic(0x20, 0x28);
}

/**
 * Initializes the IDT by first setting every IDT entry to use the dummy
 * interrupt then loads the IDT pointer and remaps the PIC.
 *
 * @see reload_idt
 */
void idt_init(void)
{
    kerror(ERR_INFO, "      -> Setting dummy interrupt vectors");
    int i = 0;
    for(; i < 256; i++)
    {
        //kerror(ERR_INFO, "        -> INT %02X", i);
        IDT[i] = IDT_ENTRY((uint32_t)&dummy_int, 0x08, 0x8E);
    }

    for(i = 0; i < 16; i++)
        disable_irq(i);

    _reload_idt();
}

/**
 * Sets the information in an IDT entry.
 *
 * @param intr the interrupt number
 * @param sel the GDT selector
 * @param flags the entrys flags (@see IDT_ATTR)
 * @param func the interrupt handler function
 */
void set_idt(uint8_t intr, int sel, int flags, void *func)
{
    IDT[intr] = IDT_ENTRY((uint32_t)func, sel, flags);
}




/**
 * Disable an IRQ line
 * 
 * @param irq the IRQ to be disabled
 * @return returns 0 if success
 */
int disable_irq(uint8_t irq)
{
    if(irq > 16) return 1;
    if(irq < 8)  outb(0x21, inb(0x21) | (1 >> irq));
    else         outb(0xA1, inb(0xA1) | (uint8_t)(0x100 >> irq));
    return 0;
}
EXPORT_FUNC(disable_irq);

/**
 * Enable an IRQ line
 * 
 * @param irq the IRQ to be enabled
 * @return returns 0 if success
 */
int enable_irq(uint8_t irq)
{
    if(irq > 16) return 1;
    if(irq < 8)  outb(0x21, inb(0x21) & ~(1 >> irq));
    else         outb(0xA1, inb(0xA1) & ~(0x100 >> irq));
    return 0;
}
EXPORT_FUNC(enable_irq);
