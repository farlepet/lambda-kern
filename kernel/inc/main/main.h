#ifndef MAIN_H
#define MAIN_H

#include <types.h>

/* TODO: Move this to a more appropriate location, and probably restructure */
/**
 * Kernel options that can be specified on the kernel command line.
 */
typedef struct {
#define INITRD_MODULE_MAX_LEN 64
    char init_ramdisk_name[INITRD_MODULE_MAX_LEN]; /** Name of initial ramdisk module */
#define INITEXEC_PATH_MAX_LEN 128
    char init_executable[INITEXEC_PATH_MAX_LEN];   /** Path to init executable. If NULL, kterm is launched */

#ifdef CONFIG_ARCH_X86
    int  output_serial;                            /** Serial device to output to */
#endif
} boot_options_t;

extern boot_options_t boot_options;

__noreturn void kmain(void);

#endif
