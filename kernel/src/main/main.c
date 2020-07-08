#include <proc/syscalls.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <multiboot.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <fs/initrd.h>
#include <intr/intr.h>
#include <fs/fs.h>

// Architecture-specific initialization:
#include <init/init.h>

__noreturn void kernel_task(void);
__noreturn int kmain(struct multiboot_header *, uint32_t);

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
__noreturn int kmain(struct multiboot_header *mboot_head, uint32_t magic)
{
	if(magic != 0x2BADB002)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);

	check_commandline(mboot_head);

	// Architecture-specific initialization:
	arch_init(mboot_head);

	
	fs_init();
	initrd_init(mboot_head);
	
	timer_init(100);

	init_syscalls();

	init_multitasking(&kernel_task, "kern");

	iloop();
}

/**
 * The main kernel task, spawns a few other tasks, then busy-waits.
 */
__noreturn void kernel_task()
{
	kerror(ERR_BOOTINFO, "Main kernel task started");

	init_ktasks();

	for(;;) busy_wait();
}
