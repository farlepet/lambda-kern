#include <lambda/export.h>
#include <intr/intr.h>
#include <err/error.h>

#include <arch/init/hw_init.h>
#include <arch/init/init.h>
#include <arch/intr/int.h>

void interrupt_attach(interrupt_idx_e n, intr_handler_hand_t *hdlr) {
    arch_interrupt_attach(n, hdlr);
    kerror(ERR_DEBUG, "Interrupt vector 0x%02X set", n);
}
EXPORT_FUNC(interrupt_attach);

void timer_init(uint32_t quantum) {
    hw_init_timer(quantum);
    
    kerror(ERR_INFO, "Timer initialized");
}

