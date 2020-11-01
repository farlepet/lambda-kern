#ifndef SERIAL_H
#define SERIAL_H

#include <types.h>

#include <hal/io/char/char.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

int serial_create_chardev(uint16_t port, hal_io_char_dev_t *chardev);

/**
 * \brief Initialize the serial port.
 * Initialize the serial port.
 * @param port which port to initialize
 */
void serial_init(uint16_t port);

/**
 * \brief Check if a byte is waiting to be read.
 * Check if a byte is waiting to be read.
 * @param port serial port to check
 */
int serial_received(uint16_t port);

/**
 * \brief Reads a byte from the serial port.
 * Reads a byte from the specified serial port.
 * @param port serial port to read from
 * @see serial_received
 */
char serial_read(uint16_t port);

/**
 * \brief Checks if it is okay to send a byte to the serial port.
 * Checks if it is okay to send a byte to the specified serial port.
 * @param port serial port to check
 */
int is_transmit_empty(uint16_t port);

/**
 * \brief Writes a byte to a serial port.
 * Writes a byte to the specified serial port.
 * @param port the serial port to write to
 * @param a the byte to write to the port
 * @see is_transmit_empty
 */
void serial_write(uint16_t port, char a);




/**
 * @brief Write string to specified serial port
 * 
 * @param port Serial port to write to
 * @param str NULL-terminated string to write to port
 */
void serial_print(uint16_t port, char *str);

/**
 * @brief Conver number to string and write to specified serial port
 * 
 * @param port Port to write to
 * @param n Number to convert to string
 * @param base Base in which to interpret number
 */
void serial_printnum(uint16_t port, uint32_t n, int base);

#endif