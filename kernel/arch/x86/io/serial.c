#include <types.h>
#include <io/ioport.h>

/**
 * \brief Initialize the serial port.
 * Initialize the serial port.
 * @param port which port to initialize
 */
void serial_init(u16 port)
{
	outb(port + 1, 0x00);
	outb(port + 3, 0x80);
	outb(port + 0, 0x03);
	outb(port + 1, 0x00);
	outb(port + 3, 0x03);
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);
}

/**
 * \brief Check if a byte is waiting to be read.
 * Check if a byte is waiting to be read.
 * @param port serial port to check
 */
int serial_received(u16 port)
{
	return inb(port + 5) & 1;
}

/**
 * \brief Reads a byte from the serial port.
 * Reads a byte from the specified serial port.
 * @param port serial port to read from
 * @see serial_received
 */
char serial_read(u16 port)
{
	while (serial_received(port) == 0);
	
	return (char)inb(port);
}

/**
 * \brief Checks if it is okay to send a byte to the serial port.
 * Checks if it is okay to send a byte to the specified serial port.
 * @param port serial port to check
 */
int is_transmit_empty(u16 port)
{
	return inb(port + 5) & 0x20;
}

/**
 * \brief Writes a byte to a serial port.
 * Writes a byte to the specified serial port.
 * @param port the serial port to write to
 * @param a the byte to write to the port
 * @see is_transmit_empty
 */
void serial_write(u16 port, char a)
{
	while (is_transmit_empty(port) == 0);
	outb(port, (u8)a);
}