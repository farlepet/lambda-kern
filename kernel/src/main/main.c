#include <multiboot.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <err/error.h>
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
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);
	
	
	mm_init(mboot_tag);
	
	interrupts_init();

	timer_init(1000); // At this speed, the counter will roll over after 5997302.87 centuries
	
	kerror(ERR_BOOTINFO, 1, "Lambda OS kernel finished initializing");
	
	for(;;);
}