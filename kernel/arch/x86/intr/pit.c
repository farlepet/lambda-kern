#include <proc/mtask.h>
#include <io/ioport.h>
#include <intr/intr.h>
#include <time/time.h>
#include "idt.h"
#include "pit.h"

extern void pit_int(); //!< The PIT interrupt handler
void pit_handler(void); //!< Assembly PIT interrupt handler

/**
 * \brief PIT interrupt handler.
 * The main part of the PIT interrupt handler, called from pit_int().
 * @see pit_int
 */
void pit_handler()
{
	kerneltime++;
	
	register uint32_t i = 0;
	for(; i < MAX_TIME_BLOCKS; i++)
		if(time_blocks[i].event)
			if(--time_blocks[i].count == 0x00000000)
				do_time_block_timeup(i);
	
	outb(0x20, 0x20);
	
	//do_task_switch();
}

/**
 * \brief Creates a PIT reload value.
 * Creates a PIT reload value from the specified frequency.
 * @param freq frequency in Hz
 */
static __inline uint32_t get_reload(uint32_t freq)
{
	if(freq < 18)      return 0x10000;   // Is the frequency too small?
	if(freq > 1193181) return 0x01;     // Is the frequency too large?
	return (1193180 / freq);           // If not, compute the reload value
}

/**
 * \brief Initialize the PIT.
 * Initialized the PIT using the supplied frequency if possible.
 * @param freq frequency in Hz
 */
void pit_init(uint32_t freq)
{
	uint32_t reload = get_reload(freq);
	outb(0x43, 0x34);
	outb(0x40, (uint8_t)reload);
	outb(0x40, (uint8_t)(reload >> 8));
	set_interrupt(TIMER_INT, &pit_int);
	enable_irq(0);
}
