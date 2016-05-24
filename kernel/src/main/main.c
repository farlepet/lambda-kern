// vim: ts=4 sw=4
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

#ifdef ARCH_X86
// TODO: Move these to another file:
#include <io/pci.h>
#include <io/serial.h>
#include <dev/keyb/input.h>
#include <dev/vga/print.h>
#endif

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

#ifdef ARCH_X86
	vga_clear();
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

	pci_enumerate();
	pci_init();


	u32 args[4];

	args[0] = KVID_TASK_SLOT;
	args[1] = 50;
	//int kvid = get_ktask(KVID_TASK_SLOT, 50);
	call_syscall(SYSCALL_GET_KTASK, args);
	int kvid = args[0];
	if(kvid) // A quick message-passing test
	{
		struct kvid_print_m kpm;
		kpm.ktm.pid    = current_pid;
		kpm.ktm.type   = KVID_PRINT;
		kpm.kpm.string = "Hello via kvid!\n";
		send_message(kvid, &kpm, sizeof(struct kvid_print_m));
	}

	//fs_debug(8);
/*
	kerror(ERR_BOOTINFO, "Opening test.elf");

	struct kfile *elf = fs_finddir(fs_root, "test.elf");
	if(elf)
	{
		kerror(ERR_BOOTINFO, "  -> Found test.elf!");
		u32 size = elf->length;
		u8 *elfd = kmalloc(size);
		fs_read(elf, 0, size, elfd);
		u32 *pdir;
		ptr_t elfe = load_elf(elfd, size, &pdir);
		if(!elfe) goto ELF_ERR_BRA;

		kerror(ERR_BOOTINFO, "  -> Entrypoint: 0x%08X", elfe);
		add_kernel_task_pdir((void *)elfe, "test.elf", 0x1000, PRIO_DRIVER, pdir);
		//add_kernel_task((void *)elfe, "test.elf", 0x1000, PRIO_KERNEL);
		kerror(ERR_BOOTINFO, "      -> Created task");
	}
	else
	{
		kerror(ERR_BOOTINFO, "  -> Could not find test.elf!");
	}

ELF_ERR_BRA:
*/

	for(;;) busy_wait();
}
