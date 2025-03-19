#include <errno.h>
#include <string.h>

#include <arch/io/uart/uart_pl011.h>

#include <proc/ktasks.h>
#include <err/error.h>

static input_dev_t _serial_dev = {0};

#define SERIAL_BUFF_CNT 16
static cbuff_t _serial_buff = STATIC_CBUFF(sizeof(struct input_event) * SERIAL_BUFF_CNT);

static void _chardev_putc(void *data, int c);
static int  _chardev_getc(void *data);
static int  _chardev_chavail(void *data);

int uart_pl011_create_chardev(uart_pl011_handle_t *hand, hal_io_char_dev_t *chardev) {
    memset(chardev, 0, sizeof(hal_io_char_dev_t));
    
    chardev->data = (void *)hand;

    chardev->putc    = _chardev_putc;
    chardev->getc    = _chardev_getc;
    chardev->chavail = _chardev_chavail;

    chardev->cap = HAL_IO_CHARDEV_CAP_OUTPUT | HAL_IO_CHARDEV_CAP_INPUT;

    return 0;
}

static int _calc_divisor(uint32_t clkfreq, uint32_t baud, uint16_t *divint, uint8_t *divfrac) {
    if(clkfreq < (16 * baud)) {
        return -EINVAL;
    }

    float div = (float)clkfreq / ((float)baud * 16.0F);

    *divint  = (uint16_t)div;
    *divfrac = (uint8_t)((div - (float)(uint16_t)div) * 64.0F);

    return 0;
}

int uart_pl011_init(uart_pl011_handle_t *hand, void *base, hal_clock_dev_t *src_clock, uint32_t baud) {
    if(!base ||
       !src_clock) {
        return -EINVAL;
    }

    hand->base      = (uart_pl011_regmap_t *)base;
    hand->src_clock = src_clock;

    /* Disable */
    hand->base->CR = 0;

    /* Clear interrupts */
    hand->base->ICR = 0x7FF;

    uint16_t intdiv;
    uint8_t  fracdiv;
    /* TODO: Determine UART source clock frequency */
    if (_calc_divisor(hand->src_clock->freq, baud, &intdiv, &fracdiv)) {
        return -EUNSPEC;
    }
    hand->base->IBRD = intdiv;
    hand->base->FBRD = fracdiv;

    /* TODO: Allow for configuration here */
    hand->base->LCR = (1UL << UART_PL011_LCR_FEN__POS) |
                      (3UL << UART_PL011_LCR_WLEN__POS);

    hand->base->CR  = (1UL << UART_PL011_CR_UARTEN__POS) |
                      (1UL << UART_PL011_CR_TXE__POS)    |
                      (1UL << UART_PL011_CR_RXE__POS);

    add_input_dev(&_serial_dev, IDRIVER_SERIAL, "ser", 1, 0);
    _serial_dev.iev_buff = &_serial_buff;
 
    return 0;
}

static void _handle_input(uart_pl011_handle_t *hand, char ch) {
    struct input_event iev;
    iev.origin.s.driver = IDRIVER_SERIAL;
    iev.origin.s.device = hand->idev->id.s.device;
    iev.type = EVENT_CHAR;
    iev.data = ch;
    cbuff_write((uint8_t *)&iev, sizeof(struct input_event), _serial_dev.iev_buff);
}

static void _intr_recv_handler(uint32_t int_n, void *data) {
    (void)int_n;

    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;

    uint8_t inp;
    while(!(hand->base->FR & (1UL << UART_PL011_FR_RXFE__POS))) {
        inp = (uint8_t)hand->base->DR;
        _handle_input(hand, inp);
    }

    /* Clear all UART interrupts */
    hand->base->ICR = 0x07FF;
}

int uart_pl011_int_attach(uart_pl011_handle_t *hand, hal_intctlr_dev_t *intctlr, uint32_t int_n) {
    hal_intctlr_dev_intr_attach(intctlr, int_n, _intr_recv_handler, hand);
    hal_intctlr_dev_intr_enable(intctlr, int_n);

    hand->base->CR &= ~(1UL << UART_PL011_CR_UARTEN__POS);

    /* Interrupt at FIFO 1/8 full */
    hand->base->IFLS = (0UL << UART_PL011_IMSC_TXIM__POS) |
                       (0UL << UART_PL011_IMSC_RXIM__POS);

    /* Enable RX interrupt */
    hand->base->IMSC = (1UL << UART_PL011_IMSC_RXIM__POS);

    hand->base->CR |= (1UL << UART_PL011_CR_UARTEN__POS);

    return 0;
}

static void _chardev_putc(void *data, int c) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;

    while(hand->base->FR & (1UL << UART_PL011_FR_TXFF__POS));
    hand->base->DR = (uint8_t)c;
}

static int _chardev_getc(void *data) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;
    
    while(hand->base->FR & (1UL << UART_PL011_FR_RXFE__POS));
    return (int)(hand->base->DR & 0xFF);
}

static int _chardev_chavail(void *data) {
    uart_pl011_handle_t *hand = (uart_pl011_handle_t *)data;
    
    return !(hand->base->FR & (1UL << UART_PL011_FR_RXFE__POS));
}
