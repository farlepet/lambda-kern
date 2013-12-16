#include "pit.h"
#include <io/ioport.h>
#include "idt.h"
#include <intr/intr.h>
#include <time/time.h>

extern void pit_int();

u64 kernel_time;



void pit_handler()
{
	kernel_time++;
	
	register u32 i = 0;
	for(; i < MAX_TIME_BLOCKS; i++)
		if(time_blocks[i].event)
			if(--time_blocks[i].count == 0x00000000)
				do_time_block_timeup(i);
}

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
	set_interrupt(TIMER_INT, &pit_int);
	enable_irq(0);
}