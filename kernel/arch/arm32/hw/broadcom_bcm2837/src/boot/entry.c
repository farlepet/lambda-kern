#include <types.h>
#include <string.h>

#include <main/main.h>
#include <err/panic.h>
#include <fs/fs.h>

#include <arch/init/init.h>

__noreturn void kentry(uint32_t board, uint32_t machine, void *atags);

/* Values defined in linker script */
extern uint8_t __bss_begin;
extern uint8_t __bss_end;
static void _data_init(void) {
    memset(&__bss_begin, 0, (&__bss_end - &__bss_begin));
}

/**
 * Kernel entry-point: performs target-specific system initialization.
 */
__noreturn void kentry(uint32_t board, uint32_t machine, void *atags) {
    (void)board;
    (void)machine;
    (void)atags;

    _data_init();

    arch_init();
	
    fs_init();

    kmain();
}
