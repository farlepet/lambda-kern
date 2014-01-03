#include <proc/ktasks.h>
#include <intr/intr.h>
#include <io/ioport.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <intr/idt.h>
#include <video.h>
#include "input.h"

extern void keyb_int(); //!< Assembly interrupt handler

static const char key_table[256] = //!< Corresponds certain keys to certain characters
{
//    0     1   2    3    4    5    6    7    8    9    A    B    C    D     E     F
      0 ,0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',     //0
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a',  's',    //1
	 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',  0 ,'\\', 'z', 'x',  'c',  'v',   //2
	 'b', 'n', 'm', ',', '.', '/',  0,   0,   0,  ' ',  0,   0,   0,   0,    0,    0,   //3
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 ,  0,    0,    0,  //4
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,                                 //5
};

static const char key_shift_table[256] = //!< Corresponds certain keys to certain characters, when shift or capslock in enabled
{
//    0     1   2    3    4    5    6    7    8    9    A    B    C    D     E     F
      0, 0x1B, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',     //0
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A',  'S',    //1
	 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',  0,  '|', 'Z', 'X',  'C',  'V',   //2
	 'B', 'N', 'M', '<', '>', '?',  0,   0,   0,  ' ',  0,   0,   0,   0,    0,    0,   //3
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,  //4
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,                                 //5
};

/**
 * Takes the scancode from the buffer, and converts it into another code
 * format recognizable by Lambda OS.
 * 
 * TODO: Add more functionality to the driver
 * 
 * @param keycode key code passed in by assembly interrupt handler
 */
void keyb_handle(u32 keycode)
{
	// Doesn't do a whole lot... YET...
	char ch = key_table[keycode];
	if(ch)
	{
		//kprintf("%c", ch);
		if(ktask_pids[KINPUT_TASK_SLOT])
		{
			send_message(ktask_pids[KINPUT_TASK_SLOT], &ch, sizeof(char)); // TODO: Use actual structure instead of 8-bit character
		}
	}
}


static inline void kbd_wait(void)
{
	asm("1:   inb   $0x64,%al\n"
		"testb   $0x02,%al\n"
		"jne   1b");
}

void keyb_init()
{
	inb(0x60);
	kbd_wait();
	outb(0x60, 0xFF);
	kbd_wait();
	u8 val = 0;
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

	set_interrupt(KEYBOARD_INT, &keyb_int);
	enable_irq(1);
}