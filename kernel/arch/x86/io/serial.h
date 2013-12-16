#ifndef SERIAL_H
#define SERIAL_H

#include <types.h>

#define SERIAL_COM1 0x3f8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

/**
 * \brief Initialize the serial port.
 * Initialize the serial port.
 * @param port which port to initialize
 */
void serial_init(u16 port);

/**
 * \brief Check if a byte is waiting to be read.
 * Check if a byte is waiting to be read.
 * @param port serial port to check
 */
int serial_received(u16 port);

/**
 * \brief Reads a byte from the serial port.
 * Reads a byte from the specified serial port.
 * @param port serial port to read from
 * @see serial_received
 */
char serial_read(u16 port);

/**
 * \brief Checks if it is okay to send a byte to the serial port.
 * Checks if it is okay to send a byte to the specified serial port.
 * @param port serial port to check
 */
int is_transmit_empty(u16 port);

/**
 * \brief Writes a byte to a serial port.
 * Writes a byte to the specified serial port.
 * @param port the serial port to write to
 * @param a the byte to write to the port
 * @see is_transmit_empty
 */
void serial_write(u16 port, char a);

#endif