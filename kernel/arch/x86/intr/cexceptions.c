#include <mm/paging.h>
#include <io/serial.h>
#include <types.h>

void handle_page_fault()
{
	u32 cr2; asm ("mov %%cr2, %0" : "=r" (cr2));

	serial_print(SERIAL_COM1, " At 0x");
	serial_printnum(SERIAL_COM1, cr2, 16);
	serial_print(SERIAL_COM1, "\n");

	map_page((void *)cr2, (void *)cr2, 3);
}