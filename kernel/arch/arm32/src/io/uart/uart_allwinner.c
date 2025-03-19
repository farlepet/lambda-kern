/* NOTE: Currently supports Allwinner V3s compatible UART structures only */

#include <errno.h>
#include <string.h>

#include <arch/io/uart/uart_allwinner.h>

static void chardev_putc(void *data, int c);
static int chardev_getc(void *data);
static int chardev_chavail(void *data);

int uart_allwinner_create_chardev(uart_allwinner_handle_t *hand, hal_io_char_dev_t *chardev) {
    memset(chardev, 0, sizeof(hal_io_char_dev_t));
    
    chardev->data = (void *)hand;

    chardev->putc    = chardev_putc;
    chardev->getc    = chardev_getc;
    chardev->chavail = chardev_chavail;

    chardev->cap = HAL_IO_CHARDEV_CAP_OUTPUT | HAL_IO_CHARDEV_CAP_INPUT;

    return 0;
}

int uart_allwinner_init(uart_allwinner_handle_t *hand, void *base, uint32_t baud) {
    if(!base) {
        return -EINVAL;
    }

    hand->base = (uart_allwinner_regmap_t *)base;

    hand->base->LCR |= (1UL << UART_ALLWINNER_LCR_DLAB__POS);

    /* Baud rate = sclk / (div * 16) */
    /* Need to ungate UART1 in BUS_CLK_GATING_REG3 */
    (void)baud;

    return 0;
}

static void chardev_putc(void *data, int c) {
    __unused
    uart_allwinner_handle_t *hand = (uart_allwinner_handle_t *)data;
    (void)c;
}

static int chardev_getc(void *data) {
    __unused
    uart_allwinner_handle_t *hand = (uart_allwinner_handle_t *)data;
    
    return 0;
}

static int chardev_chavail(void *data) {
    __unused
    uart_allwinner_handle_t *hand = (uart_allwinner_handle_t *)data;
    
    return 0;
}
