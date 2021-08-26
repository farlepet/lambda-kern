#include <arch/dev/keyb/input.h>
#include <arch/io/ioport.h>
#include <arch/intr/idt.h>

#include <proc/ktasks.h>
#include <data/cbuff.h>
#include <intr/intr.h>
#include <err/error.h>
#include <io/input.h>
#include <video.h>


extern void keyb_int(); //!< Assembly interrupt handler
void keyb_handle(uint32_t);

input_dev_t keyb_dev; //!< Device struct for the keyboard input handler

#define KEYB_BUFF_CNT 16
static cbuff_t _keyb_buff = STATIC_CBUFF(sizeof(struct input_event) * KEYB_BUFF_CNT);

/**
 * Called every time a keyboard IRQ occurs. Checks to see if shift, ctrl, or alt are pressed or released
 *
 * @param keycode code that the keyboard has given us
 */
static void process_code(uint32_t keycode)
{
	switch(keycode)
	{
		case 0x2A:
		case 0x36:	keyb_dev.state |= KEYB_STATE_SHIFT;
					break;

		case 0xAA:
		case 0xB6:	keyb_dev.state &= (uint32_t)~KEYB_STATE_SHIFT;
					break;

		case 0x1D:	keyb_dev.state |= KEYB_STATE_CTRL;
					break;

		case 0x9D:	keyb_dev.state &= (uint32_t)~KEYB_STATE_CTRL;
					break;

		case 0x38:	keyb_dev.state |= KEYB_STATE_ALT;
					break;

		case 0xB8:	keyb_dev.state &= (uint32_t)~KEYB_STATE_ALT;
					break;
	}
}

/**
 * Takes the scancode from the buffer, and converts it into another code
 * format recognizable by Lambda OS.
 * 
 * TODO: Add more functionality to the driver
 * 
 * @param keycode key code passed in by assembly interrupt handler
 */
void keyb_handle(uint32_t keycode)
{
	// Doesn't do a whole lot... YET...
	process_code(keycode);
	
	struct input_event iev;
	iev.origin.s.driver = IDRIVER_KEYBOARD;
	iev.origin.s.device = keyb_dev.id.s.device;
	iev.type = EVENT_KEYPRESS;
	iev.data = keycode;
	cbuff_write((uint8_t *)&iev, sizeof(struct input_event), keyb_dev.iev_buff);
}

/**
 * Waits so writing to keyboard I/O ports too fast doesn't cause a problem
 */
static inline void kbd_wait(void)
{
	asm("1: inb $0x64,%al\n"
		"testb  $0x02,%al\n"
		"jne    1b");
}

/**
 * Initializes the keyboard.
 *  * Checks that the keyboard is in working condition
 *  * Sets the keyboard interrupt handler
 *  * Enables the keyboard IRQ
 *  * Creates and adds an input device driver entry corresponding to this keyboard
 */
void keyb_init()
{
	inb(0x60);
	kbd_wait();
	outb(0x60, 0xFF);
	kbd_wait();
	uint8_t val = 0;
	while((val = inb(0x60)) != 0xAA)
	{
		if(val == 0xFE)
		{
			inb(0x60);
			kbd_wait();
			outb(0x60, 0xFF);
			kbd_wait();
			continue;
		}
		if(val == 0xFC || val == 0xFD)
		{
			kerror(ERR_MEDERR, "Keyboard self-test failed");
			return;
		}
	}

	set_interrupt(INTR_KEYBOARD, (void *)&keyb_int);
	enable_irq(1);

	add_input_dev(&keyb_dev, IDRIVER_KEYBOARD, "keyb", 0, 0);
	keyb_dev.iev_buff = &_keyb_buff;
}
