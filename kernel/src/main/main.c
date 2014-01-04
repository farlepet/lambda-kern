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

	initrd_init(mboot_tag, "initrd");



	
	keyb_init();

	timer_init(500);
	
	pci_enumerate();

	pci_init();
	
	init_multitasking(&kernel_task, "kern");

	kerror(ERR_BOOTINFO, "Lambda OS kernel finished initializing");

	for(;;) busy_wait();
	
	(void)mboot_tag;
}

__noreturn void kernel_task()
{
	kerror(ERR_BOOTINFO, "Main kernel task started");

	init_ktasks();


	struct kvid_type_msg ktm;
	ktm.pid  = current_pid;
	ktm.type = KVID_PRINT;
	struct kvid_print_msg kpm;
	kpm.string = "Hello, kvid!\n";
	while(!ktask_pids[KVID_TASK_SLOT]) busy_wait(); // Wait for kvid task to start
	send_message(ktask_pids[KVID_TASK_SLOT], &ktm, sizeof(struct kvid_type_msg));
	send_message(ktask_pids[KVID_TASK_SLOT], &kpm, sizeof(struct kvid_print_msg));

#ifdef DEBUGGER
	struct kbug_type_msg kbtm;
	kbtm.pid  = current_pid;
	kbtm.type = KBUG_PROCINFO;
	struct kbug_proc_msg kb••••••••pm;
	kbpm.pid  = 0;
	kbpm.type =	KBUG_PROC_NPROCS;
	kbpm.info = 0;
	while(!ktask_pids[KBUG_TASK_SLOT]) busy_wait(); // Wait for kbug task to start
	send_message(ktask_pids[KBUG_TASK_SLOT], &kbtm, sizeof(struct kbug_type_msg));
	send_message(ktask_pids[KBUG_TASK_SLOT], &kbpm, sizeof(struct kbug_proc_msg));
	int nprocs;
	recv_message(&nprocs, sizeof(int));
	kerror(ERR_BOOTINFO, "\e[33mKbug\e[39m reports %d running processes", nprocs);
#endif

	for(;;) busy_wait();
}