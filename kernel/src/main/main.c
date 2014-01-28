// vim: ts=4 sw=4
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <multiboot.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <fs/initrd.h>
#include <proc/elf.h>
#include <proc/ipc.h>
#include <mm/mm.h>
#include <video.h>


// TODO: Move these to another file:
#include <io/pci.h>
#include <io/serial.h>
#include <dev/keyb/input.h>

void kernel_task(void);
__noreturn int kmain(struct multiboot_header *, u32);

__noreturn static void iloop()
{
	kerror(ERR_BOOTINFO, "iloop()");
	for(;;) busy_wait();
}

/**
 * Main kernel functions, initializes all devices, and sets up environment.
 * 
 * @param mboot_head pointer to multiboot structure
 * @param magic magic number telling us this is a multiboot-compliant bootloader
 */
int kmain(struct multiboot_header *mboot_head, u32 magic)
{
	if(magic != 0x2BADB002)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);

	serial_init(SERIAL_COM1);

	check_commandline(mboot_head);

	
	kerror(ERR_BOOTINFO, "Kernel occupies this memory space: %08X - %08X", &kern_start, &kern_end);


	mm_init(mboot_head);

	interrupts_init();

	// TODO: Create root `/` directory somehow
	initrd_init(mboot_head);
	
	keyb_init();

	timer_init(500);
	
	pci_enumerate();

	pci_init();


	init_multitasking(&kernel_task, "kern");

	kerror(ERR_BOOTINFO, "Lambda OS kernel finished initializing");

	iloop();
}

/**
 * The main kernel task, spawns a few other tasks, then busy-waits.
 */
__noreturn void kernel_task()
{
	kerror(ERR_BOOTINFO, "Main kernel task started");

	init_ktasks();


	int kvid = get_ktask(KVID_TASK_SLOT, 50);
	if(kvid) // A quick message-passing test
	{
		struct kvid_print_m kpm;
		kpm.ktm.pid    = current_pid;
		kpm.ktm.type   = KVID_PRINT;
		kpm.kpm.string = "Hello kernel via kvid!\n";
		send_message(kvid, &kpm, sizeof(struct kvid_print_m));
	}

	u32 elf_size;
	void *elf = initrd_find_file("test.elf", &elf_size);
	load_elf(elf, elf_size);

	for(;;) busy_wait();
}
