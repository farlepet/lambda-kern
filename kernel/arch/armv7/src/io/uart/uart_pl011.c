#include <string.h>

#include <arch/io/uart/uart_pl011.h>

static void chardev_putc(void *data, int c);
static int  chardev_getc(void *data);
static int  chardev_chavail(void *data);

int uart_pl011_create_chardev(uart_pl011_handle_t *hand, hal_io_char_dev_t *chardev) {
    memset(chardev, 0, sizeof(hal_io_char_dev_t));
    
    chardev->data = (void *)hand;

    chardev->putc    = chardev_putc;
    chardev->getc    = chardev_getc;
    chardev->chavail = chardev_chavail;

    chardev->cap = HAL_IO_CHARDEV_CAP_OUTPUT | HAL_IO_CHARDEV_CAP_INPUT;

    return 0;
}

static int calc_divisor(uint32_t clkfreq, uint32_t baud, uint16_t *divint, uint8_t *divfrac) {
    if(clkfreq > (16 * baud)) {
        return -1;
    }

    float div = (float)clkfreq / ((float)baud * 16.0);

    *divint  = (uint16_t)div;
    *divfrac = (uint8_t)((div - (float)(uint16_t)div) * 64.0);

    return 0;
}

int uart_pl011_init(uart_pl011_handle_t *hand, void *base, uint32_t baud) {
    if(!base) {
        return -1;
    }

    hand->base = (uart_pl011_regmap_t *)base;

    uint16_t intdiv;
    uint8_t  fracdiv;
    /* TODO: Determine UART source clock frequency */
    if (calc_divisor(8000000, baud, &intdiv, &fracdiv)) {
        return -1;
    }
    hand->base->IBRD = intdiv;
    hand->base->FBRD = fracdiv;

    /* TODO: Allow for configuration here */
    hand->base->LCR = (1UL << UART_PL011_LCR_BRK__POS) |
                      (1UL << UART_PL011_LCR_PEN__POS) |
                      (1UL << UART_PL011_LCR_EPS__POS) |
                      (1UL << UART_PL011_LCR_FEN__POS) |
                      (3UL << UART_PL011_LCR_WLEN__POS);

    hand->base->CR  = (1UL << UART_PL011_CR_UARTEN__POS) |
                      (1UL << UART_PL011_CR_TXE__POS)    |
                      (1UL << UART_PL011_CR_RXE__POS);

    return 0;
}

static void chardev_putc(void *data, int c) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;

    while(hand->base->FR & (1UL << UART_PL011_FR_TXFF__POS));
    hand->base->DR = (uint8_t)c;
}

static int chardev_getc(void *data) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;
    
    while(hand->base->FR & (1UL << UART_PL011_FR_RXFE__POS));
    return (int)(hand->base->DR & 0xFF);
}

static int chardev_chavail(void *data) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;
    
    return !(hand->base->FR & (1UL << UART_PL011_FR_RXFE__POS));
}