// vim: ts=4 sw=4
#include <mm/stack_trace.h>
#include <proc/syscalls.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <multiboot.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <fs/initrd.h>
#include <mm/alloc.h>
#include <proc/elf.h>
#include <proc/ipc.h>
#include <mm/mm.h>
#include <fs/fs.h>
#include <video.h>
#include <string.h>

#include <fs/stream.h>

#ifdef ARCH_X86
// TODO: Move these to another file:
#include <io/serial.h>
#include <dev/keyb/input.h>
#include <dev/vga/print.h>
#endif

__noreturn void kernel_task(void);
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
__noreturn int kmain(struct multiboot_header *mboot_head, u32 magic)
{
	if(magic != 0x2BADB002)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);

#ifdef ARCH_X86
	vga_clear();
	disable_interrupts();
#endif

	serial_init(SERIAL_COM1);

	check_commandline(mboot_head);

	kerror(ERR_BOOTINFO, "Kernel occupies this memory space: %08X - %08X", &kern_start, &kern_end);
	
	mm_init(mboot_head);

	interrupts_init();

	serial_init(SERIAL_COM1); // Initialize it a second time to enable it's interrupts

	fs_init();
	initrd_init(mboot_head);
	
	keyb_init();

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
