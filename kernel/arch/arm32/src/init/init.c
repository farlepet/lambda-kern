#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/mm/mmu.h>

#include <arch/init/hw_init.h>

#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>


void arch_init(void) {
    disable_interrupts();
    disable_fiqs();

    /* Alloy for early exception debugging */
    __WRITE_VBAR(0xC0008000);

    armv7_mmu_init();
    hw_init_mm();

    if(hw_init_console()) {
        /* Of course, if we fail to initialize the console, this message may
         * not be visible/transmitted. */
        kpanic("Could not initialize kernel console!");
    }
    kerror(ERR_INFO, "Console Initialized");

    intr_init();
    hw_init_interrupts();
    kerror(ERR_INFO, "Interrupts Initialized");
}