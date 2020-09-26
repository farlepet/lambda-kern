#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>
#include <arch/intr/gic.h>
#include <arch/io/uart/uart_pl011.h>

#include <arch/registers.h>

#include <err/error.h>
#include <video.h>

uart_pl011_handle_t pl011;
hal_io_char_dev_t   uart;

armv7_gic_handle_t gic;
hal_intctlr_dev_t  intctlr;

/* NOTE: For testing only */
uint32_t scr;

void arch_init(struct multiboot_header *mboot_head) {
    (void)mboot_head;

    disable_interrupts();
    disable_fiqs();

    __READ_SCR(scr);

    /* TODO: Make this configurable and abstractable */
    uart_pl011_init(&pl011, UART_PL011_VEXPRESS_A9_UART0_BASE, 115200);
    uart_pl011_create_chardev(&pl011, &uart);
    kput_char_dev = &uart;

    kerror(ERR_BOOTINFO, "UART Initialized");

    intr_init();
    kerror(ERR_BOOTINFO, "Interrupts Initialized");

    armv7_mpcore_regmap_t *mpcore;
    __READ_PERIPHBASE(mpcore);

    armv7_gic_init(&gic, &mpcore->ICC, &mpcore->DCU);
    armv7_gic_create_intctlrdev(&gic, &intctlr);

    intr_attach_gic(&gic);
}