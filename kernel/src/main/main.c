#include <multiboot.h>
#include <intr/intr.h>
#include <kernel/arch/x86/io/ioport.h>
#include <kernel/arch/x86/io/serial.h>
#include <mm/mm.h>
#include <video.h>

extern void inttest(); //!< Simple function to test interrupts. \deprecated This is for testing only, it will be removed very soon.

/**
 * \brief Main kernel function.
 * Initializes all devices, and sets up environment.
 * @param mboot_ptr pointer to multiboot structure
 * @param initial_stack location of the initial stack
 */
int kmain(struct multiboot_header_tag *mboot_tag, u32 magic)
{
	if(magic != 0x36d76289) for(;;);
	
	mm_init(mboot_tag);
	
	interrupts_init();

	timer_init(100);

	kprint("Welcome to Lambda OS!\n");
	
	for(;;);
}


/**
 * \brief A test function for the IDT.
 * \deprecated This is for testing only, it will be removed very soon.
 */
void __unused test_int()
{
	kprint("I see the interrupts are working just fine!\n");
	outb(0x20, 0x20);
	inb(0x60);
}