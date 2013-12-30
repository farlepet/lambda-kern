#include <proc/mtask.h>
#include <multiboot.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <mm/mm.h>
#include <video.h>



#include <io/pci.h>
#include <io/serial.h>
#include <dev/keyb/input.h>
#include <intr/int.h>

void kloop1();
void kloop2();
void kloop3();

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

	serial_init(SERIAL_COM1);

	check_commandline(mboot_tag);

	mm_init(mboot_tag);

	interrupts_init();
	
	keyb_init();

	timer_init(1000);
	
	pci_enumerate();

	pci_init();
	
	init_multitasking(&kloop1, "Kernel Loop 1");

	kerror(ERR_BOOTINFO, "Lambda OS kernel finished initializing");

	for(;;);
	
	(void)mboot_tag;
}



void kloop1()
{
	add_kernel_task(&kloop2, "Kernel Loop 2");
	for(;;)
	{
		delay(1);
		kput('A');
	}
}

void kloop2()
{
	add_kernel_task(&kloop3, "Kernel Loop 3");
	for(;;)
	{
		delay(1);
		kput('B');
	}
}

void kloop3()
{
	for(;;)
	{
		delay(1);
		kput('C');
	}
}