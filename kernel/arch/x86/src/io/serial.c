#include <string.h>

#include <arch/io/ioport.h>
#include <arch/io/serial.h>
#include <arch/intr/idt.h>
#include <arch/intr/pic.h>

#include <proc/ktasks.h>
#include <data/cbuff.h>
#include <err/error.h>
#include <intr/intr.h>
#include <io/input.h>
#include <types.h>

static input_dev_t serial_dev;

#define SERIAL_BUFF_CNT 16
static cbuff_t _serial_buff = STATIC_CBUFF(sizeof(struct input_event) * SERIAL_BUFF_CNT);

static void chardev_putc(void *data, int c);
static int chardev_getc(void *data);
static int chardev_chavail(void *data);

int serial_create_chardev(uint16_t port, hal_io_char_dev_t *chardev) {
    memset(chardev, 0, sizeof(hal_io_char_dev_t));
    
    chardev->data = (void *)(uint32_t)port;

    chardev->putc    = chardev_putc;
    chardev->getc    = chardev_getc;
    chardev->chavail = chardev_chavail;

    chardev->cap = HAL_IO_CHARDEV_CAP_OUTPUT | HAL_IO_CHARDEV_CAP_INPUT;

    return 0;
}


static void handle_input(char ch) {
    struct input_event iev;
    iev.origin.s.driver = IDRIVER_SERIAL;
    iev.origin.s.device = serial_dev.id.s.device;
    iev.type = EVENT_CHAR;
    iev.data = ch;
    cbuff_write((uint8_t *)&iev, sizeof(struct input_event), serial_dev.iev_buff);
}

// TODO: Add support for all 4 serial ports
static void _serial_int_handle(intr_handler_hand_t *hdlr) {
    (void)hdlr;
    if(serial_received(SERIAL_COM1))
    {
        char ch = (char)inb(SERIAL_COM1);
        if(ch == 0x7F) ch = '\b';

        handle_input(ch);
    }
    outb(0x20, 0x20);
}

/* @todo Possibly make this dynamically allocated */
static intr_handler_hand_t _serial_int_hdlr = {
    .callback = _serial_int_handle,
    .data     = NULL
};

/**
 * \brief Initialize the serial port.
 * Initialize the serial port.
 * @param port which port to initialize
 */
void serial_init(uint16_t port) {
    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x01);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    outb(port + 1, 0x01);

    interrupt_attach(INTR_SERIALA, &_serial_int_hdlr);
    interrupt_attach(INTR_SERIALB, &_serial_int_hdlr);
    pic_irq_enable(PIC_IRQ_SERIALA);
    pic_irq_enable(PIC_IRQ_SERIALB);

    add_input_dev(&serial_dev, IDRIVER_SERIAL, "ser", 1, 0);
    serial_dev.iev_buff = &_serial_buff;
}


/**
 * \brief Check if a byte is waiting to be read.
 * Check if a byte is waiting to be read.
 * @param port serial port to check
 */
int serial_received(uint16_t port) {
    return inb(port + 5) & 1;
}

/**
 * \brief Reads a byte from the serial port.
 * Reads a byte from the specified serial port.
 * @param port serial port to read from
 * @see serial_received
 */
char serial_read(uint16_t port) {
    while (serial_received(port) == 0);
    
    return (char)inb(port);
}

/**
 * \brief Checks if it is okay to send a byte to the serial port.
 * Checks if it is okay to send a byte to the specified serial port.
 * @param port serial port to check
 */
int is_transmit_empty(uint16_t port) {
    return inb(port + 5) & 0x20;
}

/**
 * \brief Writes a byte to a serial port.
 * Writes a byte to the specified serial port.
 * @param port the serial port to write to
 * @param a the byte to write to the port
 * @see is_transmit_empty
 */
void serial_write(uint16_t port, char a) {
    while (is_transmit_empty(port) == 0);
    outb(port, (uint8_t)a);
}


static void chardev_putc(void *data, int c) {
    /* TODO: Make this configurable */
    if(c == '\n') {
        serial_write((uint16_t)(uint32_t)data, '\r');
    }
    serial_write((uint16_t)(uint32_t)data, (char)c);
}

static int chardev_getc(void *data) {
    return serial_read((uint16_t)(uint32_t)data);
}

static int chardev_chavail(void *data) {
    return serial_received((uint16_t)(uint32_t)data);
}
