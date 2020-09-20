#include <string.h>

#include <arch/io/ioport.h>
#include <arch/intr/idt.h>
#include <arch/intr/pit.h>

#include <proc/mtask.h>
#include <intr/intr.h>
#include <time/time.h>

extern void pit_int(); //!< The PIT interrupt handler
void pit_handler(void); //!< Assembly PIT interrupt handler

static void (*pit_callback)(void) = NULL;

static int timerdev_setfreq(void *data, uint32_t freq);
static int timerdev_attach(void *data, void (*callback)(void));

void pit_create_timerdev(hal_timer_dev_t *dev) {
	memset(dev, 0, sizeof(hal_timer_dev_t));

	dev->setfreq   = timerdev_setfreq;
	dev->setperiod = NULL;
	dev->attach    = timerdev_attach;

	dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;
}

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

	if(pit_callback) {
		pit_callback();
	}

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
	set_interrupt(INT_TIMER, &pit_int);
	enable_irq(0);
}

static int timerdev_setfreq(void *data, uint32_t freq) {
	(void)data;

	if(freq < 18 || freq > 1193181) return -1;

	uint32_t reload = get_reload(freq);
	outb(0x43, 0x34);
	outb(0x40, (uint8_t)reload);
	outb(0x40, (uint8_t)(reload >> 8));

	return 0;
}

static int timerdev_attach(void *data, void (*callback)(void)) {
	(void)data;
	
	if(pit_callback) {
		/* Presently only support a single callback */
		return -1;
	}

	pit_callback = callback;

	return 0;
}
