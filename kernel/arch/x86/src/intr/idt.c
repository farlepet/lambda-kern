#include <string.h>

#include <lambda/export.h>
#include <arch/io/ioport.h>
#include <arch/intr/idt.h>
#include <arch/intr/int.h>
#include <arch/intr/pic.h>

#include <types.h>
#include <err/error.h>

extern void idt_setup_exceptions(void); /**< Sets up handlers for all exception interrupts */
extern void idt_setup_handlers(void);   /**< Sets up handlers for all non-exception interrupts */

static x86_idt_handle_t _idt_hand = { 0 };

__attribute__((used))
static x86_idt_idtr_t  _idtr;

void idt_init(void) {
    memset(&_idt_hand, 0, sizeof(_idt_hand));

    kerror(ERR_INFO, "      -> Setting exception vectors");
    idt_setup_exceptions();
    kerror(ERR_INFO, "      -> Setting interrupt vectors");
    idt_setup_handlers();

    kerror(ERR_INFO, "      -> Disabling IRQs");
    for(unsigned i = 0; i < PIC_IRQ_MAX; i++) {
        pic_irq_disable(i);
    }

    /* @todo Add generic API to allow interrupts to be called, this is just a temporary hack */
    _idt_hand.idt[INTR_SYSCALL].flags = IDT_ATTR(1, 3, 0, IDT_ENTRY_TYPE_INT32);
    _idt_hand.idt[INTR_SCHED].flags   = IDT_ATTR(1, 3, 0, IDT_ENTRY_TYPE_INT32);

    kerror(ERR_INFO, "      -> Loading IDT");
    _idtr.base  = (uint32_t)&_idt_hand.idt;
    _idtr.limit = sizeof(_idt_hand.idt)-1;
    asm volatile("lidt (_idtr)");

    kerror(ERR_INFO, "      -> Remapping IRQ's");
    pic_remap(PIC_OFFSET_MASTER, PIC_OFFSET_SLAVE);
}


void idt_set_entry(uint8_t intr, uint16_t sel, uint8_t flags, void *func) {
    _idt_hand.idt[intr].offset_low  = (uint16_t)(uintptr_t)func;
    _idt_hand.idt[intr].segment     = sel;
    _idt_hand.idt[intr]._reserved   = 0;
    _idt_hand.idt[intr].flags       = flags;
    _idt_hand.idt[intr].offset_high = (uint16_t)((uintptr_t)func >> 16);
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

