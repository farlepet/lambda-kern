#include <types.h>

#define SERIAL_COM1 0x3f8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8


void serial_init(u16 port);

int serial_received(u16 port);

char serial_read(u16 port);

int is_transmit_empty(u16 port);

void serial_write(u16 port, char a);