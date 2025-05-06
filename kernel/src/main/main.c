#include <proc/ktask/kinput.h>
#include <lambda/version.h>
#include <proc/syscalls.h>
#include <proc/ktasks.h>
#include <proc/mtask.h>
#include <proc/exec.h>
#include <proc/elf.h>
#include <mod/module.h>
#include <mm/alloc.h>
#include <main/main.h>
#include <err/panic.h>
#include <err/error.h>
#include <time/time.h>
#include <intr/intr.h>
#include <fs/initrd.h>
#include <fs/stream.h>
#include <fs/fs.h>
#include <io/output.h>

#include <sys/stat.h>
#include <string.h>

boot_options_t boot_options = {
    .init_ramdisk_name = "",
#ifdef CONFIG_ARCH_X86
    .init_executable   = "/bin/linit",
#else
    /* TODO: FS not fully implemented on other platforms. */
    .init_executable   = "",
#endif
#ifdef CONFIG_ARCH_X86
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

__noreturn void kmain(void) {
    kerror(ERR_INFO, "---------------------------------------");
    kerror(ERR_INFO, "Kernel version: "LAMBDA_VERSION_STR_FULL);
    kerror(ERR_INFO, "Compiled by:    "COMPILER_VERSION);
    kerror(ERR_INFO, "Built on:       "__DATE__" at "__TIME__);
    kerror(ERR_INFO, "---------------------------------------");

#ifdef CONFIG_EMBEDDED_INITRD
    kerror(ERR_INFO, "Embedded initrd: %08X-%08X", &INITRD_START, &INITRD_END);
    kfile_t *fs_root = fs_get_root();
    initrd_mount(fs_root, (uintptr_t)&INITRD_START, &INITRD_END - &INITRD_START);
#endif

    timer_init(100);

    syscalls_init();

    disable_interrupts();

    /* TODO: SMP suport */
    sched_init(1);
    init_multitasking(&kernel_task, "kern");

    enable_interrupts();

    iloop();
}

__noreturn
static void _spawn_init(const char *path);
__noreturn
static void _exec_init(void);

#define INIT_STREAM_LEN 512
/**
 * The main kernel task, spawns a few other tasks, then busy-waits.
 */
__noreturn
void kernel_task(void)
{
    kerror(ERR_INFO, "Main kernel task started");

    init_ktasks();

    modules_preload("/etc/modules.preload");

    if(strlen((const char *)boot_options.init_executable)) {
        _spawn_init((const char *)boot_options.init_executable);
    }

    for(;;) busy_wait();
}

static const char  *_init_file = NULL;
static volatile int _run_ok    = 0;

__noreturn
static void _spawn_init(const char *path) {
    _init_file = path;

    kerror(ERR_INFO, "Loading init executable (%s)", path);
    kfile_t *file = fs_find_file(NULL, path);
    if(!file) {
        kpanic("Could not find init executable! (%s)\n", path);
    }

    /* Setup standard streams for child */
    /* TODO: These streams should eventually be replaced by pointing to a driver
     * TTY or similar, rather than being directly controlled by the kernel. */
    kerror(ERR_INFO, "Creating STDIO streams");

    kfile_t *stdin, *stdout, *stderr;

    if(!(stdin  = stream_create(INIT_STREAM_LEN))  ||
       !(stdout = stream_create(INIT_STREAM_LEN)) ||
       !(stderr = stream_create(INIT_STREAM_LEN))) {
        kpanic("init: Could not create STDIO streams!");
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
        kpanic("Could not create stream handle(s) for init!");
    }

    kerror(ERR_INFO, "Creating task");
    int pid = add_task(&_exec_init, "init", CONFIG_PROC_KERN_STACK_SIZE, PRIO_KERNEL, PROC_DOMAIN_KERNEL, NULL);
    if(pid <= 0) {
        kpanic("Failed to parse init executable or spawn task!");
    }

    struct kproc *proc = proc_by_pid(pid);
    if(!proc) {
        kpanic("Could not find spawned init process!");
    }

    /* Sort of a hack. We want to start the task in kernel mode, but we need it
     * to enter user mode after execve. */
    /* Task needs to run at least once before we change it to a user process */
    while(_run_ok != 1) { delay(1); };
    proc->domain = PROC_DOMAIN_USERSPACE;

    proc->open_files[0] = stdin_user;
    proc->open_files[1] = stdout_user;
    proc->open_files[2] = stderr_user;

    /* Route input to init process */
    kinput_dest = stdin_kern;

    /* Allow task to execute */
    _run_ok = 2;

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

__noreturn
static void _exec_init(void) {
    _run_ok = 1; /* Task has run once */

    /* Wait for _spawn_init to set us up */
    while(_run_ok != 2) { delay(1); }

    char *argv[] = { "init", NULL }; /* TODO */
    char *envp[] = { NULL };
    execve(_init_file, (const char **)argv, (const char **)envp);

    kpanic("execve failed!");
}
