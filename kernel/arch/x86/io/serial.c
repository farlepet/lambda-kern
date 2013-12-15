#include <types.h>
#include <io/ioport.h>


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


int serial_received(u16 port)
{
	return inb(port + 5) & 1;
}

char serial_read(u16 port)
{
	while (serial_received(port) == 0);
	
	return inb(port);
}


int is_transmit_empty(u16 port)
{
	return inb(port + 5) & 0x20;
}

void serial_write(u16 port, char a)
{
	while (is_transmit_empty(port) == 0);
	outb(port, a);
}