#include <string.h>

#include <lambda/export.h>
#include <arch/io/ioport.h>
#include <arch/intr/idt.h>
#include <arch/intr/int.h>

#include <types.h>
#include <err/error.h>

extern void load_idt(uint64_t *, uint32_t); //!< Use `lidt` to set the IDT pointer.
extern void idt_setup_exceptions(void); /**< Sets up handlers for all exception interrupts */
extern void idt_setup_handlers(void);   /**< Sets up handlers for all non-exception interrupts */

static void _pic_remap(uint8_t off1, uint8_t off2);
static void _idt_reload(void);

static x86_idt_handle_t _idt_hand = { 0 };

#define IDT_SETATTR(ENTRY, ATTR) ((ENTRY) = (((ENTRY) & 0xFFFF00FFFFFFFFFFULL) | ((uint64_t)(ATTR) << 40)))

void idt_init(void) {
    memset(&_idt_hand, 0, sizeof(_idt_hand));

    kerror(ERR_INFO, "      -> Setting exception vectors");
    idt_setup_exceptions();
    kerror(ERR_INFO, "      -> Setting interrupt vectors");
    idt_setup_handlers();

    kerror(ERR_INFO, "      -> Disabling IRQs");
    for(unsigned i = 0; i < 16; i++) {
        disable_irq(i);
    }

    /* @todo Add generic API to allow interrupts to be called, this is just a temporary hack */
    IDT_SETATTR(_idt_hand.idt[INTR_SYSCALL], IDT_ATTR(1, 3, 0, int32));
    IDT_SETATTR(_idt_hand.idt[INTR_SCHED],   IDT_ATTR(1, 3, 0, int32));

    kerror(ERR_INFO, "      -> Reloading IDT");
    _idt_reload();
}


void idt_set_entry(uint8_t intr, int sel, int flags, void *func) {
    _idt_hand.idt[intr] = IDT_ENTRY((uint32_t)func, sel, flags);
}

int idt_add_callback(uint8_t int_n, intr_handler_hand_t *hdlr) {
    for(unsigned i = 0; i < X86_IDT_MAX_CALLBACKS; i++) {
        if(_idt_hand.callbacks[i].hdlr == NULL) {
            _idt_hand.callbacks[i].int_n = int_n;
            _idt_hand.callbacks[i].hdlr  = hdlr;

            return 0;
        }
    }

    return -1;
}

void idt_handle_interrupt(uint8_t int_n, x86_pusha_regs_t pregs, x86_iret_regs_t iregs) {
    for(unsigned i = 0; i < X86_IDT_MAX_CALLBACKS; i++) {
        if((_idt_hand.callbacks[i].hdlr  != NULL) &&
           (_idt_hand.callbacks[i].int_n == int_n)) {
            if(_idt_hand.callbacks[i].hdlr->callback != NULL) {
                _idt_hand.callbacks[i].hdlr->arch.iregs = &iregs;
                _idt_hand.callbacks[i].hdlr->arch.pregs = &pregs;
                _idt_hand.callbacks[i].hdlr->callback(_idt_hand.callbacks[i].hdlr);
            }
        }
    }
}




int disable_irq(uint8_t irq) {
    if(irq > 16) return 1;
    if(irq < 8)  outb(0x21, inb(0x21) | (1 >> irq));
    else         outb(0xA1, inb(0xA1) | (uint8_t)(0x100 >> irq));
    return 0;
}
EXPORT_FUNC(disable_irq);


int enable_irq(uint8_t irq) {
    if(irq > 16) return 1;
    if(irq < 8)  outb(0x21, inb(0x21) & ~(1 >> irq));
    else         outb(0xA1, inb(0xA1) & ~(0x100 >> irq));
    return 0;
}
EXPORT_FUNC(enable_irq);

/**
 * Remaps the PIC so when an IRQ fires, it adds `offx` to the IRQ number.
 * Which offset is used depends on wether the IRQ came from the master or
 * the slave PIC.
 *
 * @param off1 the offset for the master PIC
 * @param off2 the offset for the slave PIC
 */
static void _pic_remap(uint8_t off1, uint8_t off2) {
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
static void _idt_reload(void) {
    kerror(ERR_INFO, "      -> Loading IDT");
    load_idt(_idt_hand.idt, sizeof(_idt_hand.idt)-1);
    kerror(ERR_INFO, "      -> Remapping IRQ's");
    _pic_remap(0x20, 0x28);
}

