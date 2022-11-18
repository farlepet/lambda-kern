#include <types.h>

#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>

__noreturn void kentry(void);

/**
 * Kernel entry-point: performs target-specific system initialization.
 */
__noreturn void kentry(void) {
    arch_init();
    
    fs_init();

    kmain();
}
