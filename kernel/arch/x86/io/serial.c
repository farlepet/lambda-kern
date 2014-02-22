#include <proc/ktasks.h>
#include <err/error.h>
#include <io/ioport.h>
#include <io/serial.h>
#include <intr/intr.h>
#include <intr/idt.h>
#include <io/input.h>
#include <proc/ipc.h>
#include <types.h>

extern void serial_interrupt(void);

struct input_dev *serial_dev;

/**
 * \brief Initialize the serial port.
 * Initialize the serial port.
 * @param port which port to initialize
 */
void serial_init(u16 port)
{
	outb(port + 1, 0x00);
	outb(port + 3, 0x80);
	outb(port + 0, 0x01);
	outb(port + 1, 0x00);
	outb(port + 3, 0x03);
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);
	outb(port + 1, 0x01);

	set_interrupt(SERIALA_INT, &serial_interrupt);
	set_interrupt(SERIALB_INT, &serial_interrupt);
	enable_irq(4);
	enable_irq(3);

	serial_dev = add_input_dev(IDRIVER_SERIAL, "ser", 1, 0);
	if(!serial_dev)
	{
		kerror(ERR_MEDERR, "Could not set up serial device");
	}
}

static void handle_input(char ch)
{
	if(ktask_pids[KINPUT_TASK_SLOT])
	{
		struct input_event iev;
		iev.origin.s.driver = IDRIVER_SERIAL;
		iev.origin.s.device = serial_dev->id.s.device;
		iev.type = EVENT_CHAR;
		iev.data = ch;
		send_message(ktask_pids[KINPUT_TASK_SLOT], &iev, sizeof(struct input_event));
	}
}

// TODO: Add support for all 4 serial ports
void serial_int_handle()
{
	if(serial_received(SERIAL_COM1))
	{
		char ch = (char)inb(SERIAL_COM1);

		handle_input(ch);
	}
	outb(0x20, 0x20);
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
