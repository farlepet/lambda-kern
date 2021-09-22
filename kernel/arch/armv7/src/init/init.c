#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>
#include <arch/intr/gic.h>
#include <arch/io/uart/uart_pl011.h>

#include <arch/registers.h>
#include <arch/plat/platform.h>

#include <err/error.h>
#include <mm/alloc.h>
#include <video.h>

static uart_pl011_handle_t pl011;
static hal_io_char_dev_t   uart;

static armv7_gic_handle_t gic;
hal_intctlr_dev_t         intctlr;

void arch_init(void) {
    disable_interrupts();
    disable_fiqs();

    /* TODO: Make this configurable and abstractable */
    uart_pl011_init(&pl011, VEXPRESS_A9_PERIPH_UART0_BASE, 115200);
    uart_pl011_create_chardev(&pl011, &uart);
    kput_char_dev = &uart;

    kerror(ERR_INFO, "UART Initialized");

    intr_init();
    kerror(ERR_INFO, "Interrupts Initialized");

    ptr_t mpcore;
    __READ_PERIPHBASE(mpcore);
    kerror(ERR_INFO, "PERIPHBASE: %08X", mpcore);

    armv7_gic_init(&gic,
                  (void *)(mpcore + MPCORE_PERIPHBASE_OFF_ICC),
                  (void *)(mpcore + MPCORE_PERIPHBASE_OFF_DCU));
    armv7_gic_create_intctlrdev(&gic, &intctlr);
    intr_attach_gic(&gic);
    kerror(ERR_INFO, "GIC Initialized");

    uart_pl011_int_attach(&pl011, &intctlr, VEXPRESSA9_INT_UART0);

    /* TODO: Abstract this. Make memory map partially user-configurable. */
    init_alloc(VEXPRESSA9_ALLOC_BASE, VEXPRESSA9_ALLOC_SIZE);
}