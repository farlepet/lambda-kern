#include <proc/mtask.h>
#include <multiboot.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <proc/ipc.h>
#include <mm/mm.h>
#include <video.h>



#include <io/pci.h>
#include <io/serial.h>
#include <dev/keyb/input.h>

void kernel_task();

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
	
	init_multitasking(&kernel_task, "Kernel Task");

	kerror(ERR_BOOTINFO, "Lambda OS kernel finished initializing");

	for(;;);
	
	(void)mboot_tag;
}



void idle_task();
void klooptest();

u8 kbuff[17] = "ERROR";
void kernel_task()
{
	kerror(ERR_BOOTINFO, "Main kernel task started");


	kerror(ERR_BOOTINFO, "Starting idle task");
	add_kernel_task(&idle_task, "Idle Task", 0);
#if MULTITASKING_TEST
	add_kernel_task(&klooptest, "Kernel Message Test Loop", 0);
#endif

	for(;;)
	{
		recv_message(kbuff, 16);
		kerror(ERR_BOOTINFO, "kernel_task: received message: %s", kbuff);
	}
}

void idle_task()
{
	for(;;) busy_wait();
}

int n_loops;

void klooptest()
{
	int pid = current_pid;
	if(n_loops++ < 5) add_kernel_task(&klooptest, "Kernel Test Loop", 0);

	char buff[17];
	sprintf(buff, "Hello, from %d!", pid);

	send_message(-1, (u8 *)buff, 16);

	for(;;) busy_wait();
}