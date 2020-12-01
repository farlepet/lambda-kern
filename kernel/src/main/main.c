#include <lambda/version.h>
#include <proc/syscalls.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <proc/exec.h>
#include <proc/elf.h>
#include <proc/ipc.h>
#include <mm/alloc.h>
#include <multiboot.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <intr/intr.h>
#include <fs/initrd.h>
#include <fs/stream.h>
#include <fs/fs.h>
#include <video.h>

#include <sys/stat.h>
#include <string.h>
// Architecture-specific initialization:
#include <arch/init/init.h>

__noreturn void kernel_task(void);
__noreturn int kmain(struct multiboot_header *, uint32_t);

__noreturn static void iloop() {
	kerror(ERR_BOOTINFO, "iloop()");
	for(;;) busy_wait();
}

/**
 * Main kernel functions, initializes all devices, and sets up environment.
 * 
 * @param mboot_head pointer to multiboot structure
 * @param magic magic number telling us this is a multiboot-compliant bootloader
 */
__noreturn int kmain(struct multiboot_header *mboot_head, uint32_t magic) {
#if defined(ARCH_X86)
	if(magic != 0x2BADB002)
		kpanic("Invalid magic number given by the bootloader: 0x%08X", magic);
	
	check_commandline(mboot_head);
#else
	(void)magic;
#endif

	// Architecture-specific initialization:
	arch_init(mboot_head);

	kerror(ERR_BOOTINFO, "------------------------------");
	kerror(ERR_BOOTINFO, "Kernel version: "LAMBDA_VERSION_STR_FULL);
	kerror(ERR_BOOTINFO, "------------------------------");
	
	fs_init();
#if defined(ARCH_X86)
	check_multiboot_modules(mboot_head);
#endif

	timer_init(100);

	init_syscalls();

	enable_interrupts();

	//asm volatile("swi #1");

	init_multitasking(&kernel_task, "kern");

	iloop();
}

__noreturn
static void spawn_init();

#define INIT_STREAM_LEN 512
/**
 * The main kernel task, spawns a few other tasks, then busy-waits.
 */
__noreturn
void kernel_task()
{
	kerror(ERR_BOOTINFO, "Main kernel task started");

	init_ktasks();

	if(strlen((const char *)boot_options.init_executable)) {
		spawn_init();
	}

	for(;;) busy_wait();
}

__noreturn
static void spawn_init() {
	kerror(ERR_BOOTINFO, "Loading init executable (%s)", boot_options.init_executable);
	struct kfile *exec = fs_find_file(fs_root, (const char *)boot_options.init_executable);
	if(!exec) {
		kpanic("Could not open init executable! (%s)\n", boot_options.init_executable);
	}

	struct stat exec_stat;
	fs_open(exec, OFLAGS_OPEN | OFLAGS_READ);
	kfstat(exec, &exec_stat);

	void *exec_data = kmalloc(exec_stat.st_size);
	if(!exec_data) {
		kpanic("Could not allocate memory for init executable!\n");
	}

	if(fs_read(exec, 0, exec_stat.st_size, exec_data) != exec_stat.st_size) {
		kpanic("Could not read full init file into RAM!\n");
	}

	if(*(uint32_t *)exec_data != ELF_IDENT) {
		kpanic("Unsupported init executable file type!");
	}

	/* Setup standard streams for child */

	struct kfile *stdin = stream_create(INIT_STREAM_LEN);
	if(!stdin) {
		kpanic("kterm: Could not create STDIN!");
	}

	struct kfile *stdout = stream_create(INIT_STREAM_LEN);
	if(!stdout) {
		kpanic("kterm: Could not create STDOUT!");
	}

	struct kfile *stderr = stream_create(INIT_STREAM_LEN);
	if(!stderr) {
		kpanic("kterm: Could not create STDERR!");
	}

	fs_open(stdin,  OFLAGS_READ | OFLAGS_WRITE);
	fs_open(stdout, OFLAGS_READ | OFLAGS_WRITE);
	fs_open(stderr, OFLAGS_READ | OFLAGS_WRITE);

	int pid = load_elf(exec_data, exec_stat.st_size);
	if(!pid) {
		kpanic("Failed to parse init executable or spawn task!");
	}
	
	struct kproc *proc = proc_by_pid(pid);
	if(!proc) {
		kpanic("Could not find spawned init process!");
	}

	proc->open_files[0] = stdin;
	proc->open_files[1] = stdout;
	proc->open_files[2] = stderr;

	char buffer[INIT_STREAM_LEN];

	while(!(procs->type & TYPE_ZOMBIE)) {
		char t;
		struct ipc_message_user umsg;

		if(ipc_user_recv_message(&umsg) >= 0) {
			if(umsg.length > sizeof(char)) {
				ipc_user_delete_message(umsg.message_id);
			} else {
				ipc_user_copy_message(umsg.message_id, &t);
				fs_write(stdin, 0, 1, (uint8_t *)&t);
				kput(t); // TODO: Handle this better		
			}
		}

		if(stdout->length > 0) {
			int sz = fs_read(stdout, 0, stdout->length, (uint8_t *)&buffer);
			for(int i = 0; i < sz; i++) {
				kput(buffer[i]);
			}
		}

		if(stderr->length > 0) {
			int sz = fs_read(stderr, 0, stderr->length, (uint8_t *)&buffer);
			for(int i = 0; i < sz; i++) {
				kput(buffer[i]);
			}
		}
	}
		
	kpanic("Init process exited!");
}
