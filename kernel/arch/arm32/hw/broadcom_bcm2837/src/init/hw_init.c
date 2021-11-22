#include <arch/init/hw_init.h>
#include <arch/io/uart/uart_pl011.h>
#include <arch/plat/platform.h>
#include <arch/intr/bcm2835_intctlr.h>
#include <arch/io/bcm2835_gpio.h>
#include <arch/io/bcm2835_mailbox.h>

#include <err/error.h>
#include <mm/alloc.h>
#include <video.h>

static uart_pl011_handle_t pl011;
static hal_io_char_dev_t   uart;

static bcm2835_intctlr_handle_t bcm2835_intctlr;
hal_intctlr_dev_t               intctlr;

static ptr_t periphbase = 0;

static hal_clock_dev_t ioclock;

#if 0
__attribute__((aligned(16))) 
static volatile uint32_t _mbox_clkrate[12] = {
    sizeof(_mbox_clkrate), 0,
    0x38002, 12, 0, 2, 48000000,
    0,
    0, 0, 0, 0
};

__attribute__((aligned(16))) 
static volatile uint32_t _mbox_clken[12] = {
    sizeof(_mbox_clken), 0,
    0x30047, 4, 0, 2,
    0,
    0, 0, 0, 0, 0
};
#endif

int hw_init_console(void) {
    bcm2835_gpio_debug_init();
    //__READ_PERIPHBASE(periphbase);
    periphbase = (ptr_t)BROADCOM_BCM2837_PERIPHBASE_PI2;
    /* Disable UART */
    ((volatile uart_pl011_regmap_t *)(periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_PL011))->CR = 0;

    bcm2835_gpio_regmap_t *gpio =
        (bcm2835_gpio_regmap_t *)(periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_GPIO);
    bcm2835_gpio_debug(0x0);

    ioclock.freq = 48000000;

#if 0
    /* Force to 3 MHz */
    bcm2835_mailbox_regmap_t *mailbox =
        (bcm2835_mailbox_regmap_t *)(periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_MAILBOX);
    bcm2835_mailbox_write(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC, (uint32_t)&_mbox_clkrate);
    bcm2835_mailbox_read(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC);
    bcm2835_gpio_debug(0x1);
    
    bcm2835_mailbox_write(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC, (uint32_t)&_mbox_clken);
    bcm2835_mailbox_read(mailbox, BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC);
    //ioclock.freq = _mbox_clken[6];
    bcm2835_gpio_debug(0x2);
#endif

    /* Set UART pins to pulldown */
    bcm2835_gpio_setpull(gpio, 14, BCM2835_GPIO_PULL_PULLDOWN);
    bcm2835_gpio_setpull(gpio, 15, BCM2835_GPIO_PULL_PULLDOWN);
    /* Set UART pin functions to AUX0 */
    bcm2835_gpio_setfunc(gpio, 14, BCM2835_GPIO_FUNCTIONSELECT_ALT0);
    bcm2835_gpio_setfunc(gpio, 15, BCM2835_GPIO_FUNCTIONSELECT_ALT0);
    bcm2835_gpio_debug(0x3);
    
    uart_pl011_init(&pl011,
                    (void *)(periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_PL011),
                    &ioclock,
                    115200);
    bcm2835_gpio_debug(0x4);
    uart_pl011_create_chardev(&pl011, &uart);
    kput_char_dev = &uart;
    
    bcm2835_gpio_debug(0x5);

    return 0;
}

int hw_init_interrupts(void) {
    kerror(ERR_INFO, "PERIPHBASE: %08X", periphbase);

    bcm2835_intctlr_init(&bcm2835_intctlr,
                         (void *)(periphbase + BROADCOM_BCM2837_PERIPHBASE_OFF_INTCTLR));
    bcm2835_intctlr_create_intctlrdev(&bcm2835_intctlr, &intctlr);
    //intr_attach_gic(&gic);
    kerror(ERR_INFO, "GIC Initialized");

    uart_pl011_int_attach(&pl011, &intctlr, BCM2835_IRQ_UART);

    return 0;
}

/* TODO: Dynamically set base and size based on kernel size and memory capacity. */
#define ALLOC_BASE (0x00200000)
#define ALLOC_SIZE (0x00E00000)

int hw_init_mm(void) {
    /* TODO: Abstract this. Make memory map partially user-configurable. */
    init_alloc(ALLOC_BASE, ALLOC_SIZE);

    return 0;
}

/* TODO: Move */
void arch_kpanic_hook(void) {
    bcm2835_gpio_debug_init();

    for(;;) {    
        bcm2835_gpio_debug(0xAA);
        _gpio_delay_cycles(100000000);
        bcm2835_gpio_debug(0x55);
        _gpio_delay_cycles(100000000);
    }
}
