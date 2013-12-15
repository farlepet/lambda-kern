#include "pit.h"
#include <io/ioport.h>
#include "idt.h"

static __inline u32 get_reload(u32 freq)
{
	if(freq < 18)      return 0x10000;   // Is the frequency too small?
	if(freq > 1193181) return 0x01;     // Is the frequency too large?
	return (1193180 / freq);           // If not, compute the reload value
}

void pit_init(u32 freq)
{
	u32 reload = get_reload(freq);
	outb(0x43, 0x34);
	outb(0x40, (u8)reload);
	outb(0x40, (u8)(reload >> 8));
	enable_irq(0);
}