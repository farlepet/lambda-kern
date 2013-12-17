#include <multiboot.h>
#include <intr/intr.h>
#include <kernel/arch/x86/io/ioport.h>
#include <kernel/arch/x86/io/serial.h>
#include <mm/mm.h>
#include <video.h>

/**
 * \brief Main kernel function.
 * Initializes all devices, and sets up environment.
 * @param mboot_ptr pointer to multiboot structure
 * @param initial_stack location of the initial stack
 */
int kmain(struct multiboot_header_tag *mboot_tag, u32 magic)
{
	if(magic != 0x36d76289)
	{
		kprintf("Invalid magic number given by the bootloader: 0x%X", magic);
		for(;;);
	}
	
	mm_init(mboot_tag);
	
	interrupts_init();

	timer_init(100);

	kprintf("Welcome to Lambda OS!");
	
	for(;;);
}