#include <proc/ktask/kinput.h>
#include <lambda/version.h>
#include <proc/syscalls.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <proc/exec.h>
#include <proc/elf.h>
#include <mm/alloc.h>
#include <main/main.h>
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

volatile boot_options_t boot_options = {
	.init_ramdisk_name = "",
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	.init_executable   = "/bin/linit",
#else
	/* TODO: FS not fully implemented on other platforms. */
	.init_executable   = "",
#endif
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	.output_serial     = 0,
#endif
};


__noreturn void kernel_task(void);

__noreturn static void iloop() {
	kerror(ERR_INFO, "iloop()");
	for(;;) busy_wait();
}

#ifdef __clang__
#  define COMPILER "clang"
#  define COMPILER_VERSION COMPILER" "__clang_version__
#else
#  define COMPILER "GCC"
#  define COMPILER_VERSION COMPILER" "__VERSION__
#endif

/**
 * Main kernel functions, initializes all devices, and sets up environment.
 * 
 * @param mboot_head pointer to multiboot structure
 * @param magic magic number telling us this is a multiboot-compliant bootloader
 */
__noreturn void kmain(void) {
	kerror(ERR_INFO, "---------------------------------------");
	kerror(ERR_INFO, "Kernel version: "LAMBDA_VERSION_STR_FULL);
	kerror(ERR_INFO, "Compiled by:    "COMPILER_VERSION);
	kerror(ERR_INFO, "Built on:       "__DATE__" at "__TIME__);
	kerror(ERR_INFO, "---------------------------------------");

#if (FEATURE_INITRD_EMBEDDED)
	kerror(ERR_INFO, "Embedded initrd: %08X-%08X", &_binary_initrd_cpio_start, &_binary_initrd_cpio_end);
	initrd_mount(fs_root, (uintptr_t)&_binary_initrd_cpio_start, &_binary_initrd_cpio_end - &_binary_initrd_cpio_start);
#endif

	timer_init(100);

	init_syscalls();

	disable_interrupts();

	/* TODO: SMP suport */
	sched_init(1);
	init_multitasking(&kernel_task, "kern");

	enable_interrupts();

	iloop();
}

__noreturn
static void spawn_init(void);

#define INIT_STREAM_LEN 512
/**
 * The main kernel task, spawns a few other tasks, then busy-waits.
 */
__noreturn
void kernel_task(void)
{
	kerror(ERR_INFO, "Main kernel task started");

	init_ktasks();

	if(strlen((const char *)boot_options.init_executable)) {
		spawn_init();
	}

	for(;;) busy_wait();
}

__noreturn
static void spawn_init(void) {
	kerror(ERR_INFO, "Loading init executable (%s)", boot_options.init_executable);
	struct kfile *exec = fs_find_file(fs_root, (const char *)boot_options.init_executable);
	if(!exec) {
		kpanic("Could not open init executable! (%s)\n", boot_options.init_executable);
	}

	kerror(ERR_INFO, "Opening executable");

	void  *exec_data;
	size_t exec_size;

	/* TODO: Execution should all be handled by another function, or even plain execve */
	if(fs_read_file_by_path((const char *)boot_options.init_executable, NULL, &exec_data, &exec_size, 0)) {
		kpanic("Could not open %s for init!\n", boot_options.init_executable);
	}
	
	if(*(uint32_t *)exec_data != ELF_IDENT) {
		kpanic("Unsupported init executable file type!");
	}

	/* Setup standard streams for child */
	/* TODO: These streams should eventually be replaced by pointing to a driver
	 * TTY or similar, rather than being directly controlled by the kernel. */
	kerror(ERR_INFO, "Creating STDIO streams");

	kfile_t *stdin = stream_create(INIT_STREAM_LEN);
	if(!stdin) {
		kpanic("init: Could not create STDIN!");
	}

	kfile_t *stdout = stream_create(INIT_STREAM_LEN);
	if(!stdout) {
		kpanic("init: Could not create STDOUT!");
	}

	kfile_t *stderr = stream_create(INIT_STREAM_LEN);
	if(!stderr) {
		kpanic("init: Could not create STDERR!");
	}

	kfile_hand_t *stdin_kern  = fs_handle_create_open(stdin,  OFLAGS_WRITE);
	kfile_hand_t *stdout_kern = fs_handle_create_open(stdout, OFLAGS_READ);
	kfile_hand_t *stderr_kern = fs_handle_create_open(stderr, OFLAGS_READ);
	kfile_hand_t *stdin_user  = fs_handle_create_open(stdin,  OFLAGS_READ);
	kfile_hand_t *stdout_user = fs_handle_create_open(stdout, OFLAGS_WRITE);
	kfile_hand_t *stderr_user = fs_handle_create_open(stderr, OFLAGS_WRITE);
	if(!stdin_kern  ||
	   !stdout_kern ||
	   !stderr_kern ||
	   !stdin_user  ||
	   !stdout_user ||
	   !stderr_user) {
		kpanic("Could not create stream habdle(s) for init!");
	   }

	kerror(ERR_INFO, "Loading ELF");
	int pid = load_elf(exec_data, exec_size);
	if(!pid) {
		kpanic("Failed to parse init executable or spawn task!");
	}
	
	struct kproc *proc = proc_by_pid(pid);
	if(!proc) {
		kpanic("Could not find spawned init process!");
	}

	proc->open_files[0] = stdin_user;
	proc->open_files[1] = stdout_user;
	proc->open_files[2] = stderr_user;

	/* Route input to init process */
	kinput_dest = stdin_kern;

	char buffer[INIT_STREAM_LEN];

	ssize_t sz;
	while(!(proc->type & TYPE_ZOMBIE)) {
		sz = fs_read(stdout_kern, 0, INIT_STREAM_LEN, (uint8_t *)&buffer);
		for(ssize_t i = 0; i < sz; i++) {
			kput(buffer[i]);
		}

		sz = fs_read(stderr_kern, 0, INIT_STREAM_LEN, (uint8_t *)&buffer);
		for(ssize_t i = 0; i < sz; i++) {
			kput(buffer[i]);
		}

		delay(1);
	}
		
	kpanic("Init process exited!");
}
