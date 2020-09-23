#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>
#include <arch/io/uart/uart_pl011.h>

#include <err/error.h>
#include <video.h>

uart_pl011_handle_t pl011;
hal_io_char_dev_t   uart;

void arch_init(struct multiboot_header *mboot_head) {
    (void)mboot_head;

    disable_interrupts();

    /* TODO: Make this configurable and abstractable */
    uart_pl011_init(&pl011, UART_PL011_QEMU_VERSATILEPB_UART0_BASE, 115200);
    uart_pl011_create_chardev(&pl011, &uart);
    kput_char_dev = &uart;

    kerror(ERR_BOOTINFO, "UART Initialized");

    intr_init();
    kerror(ERR_BOOTINFO, "Interrupts Initialized");
}