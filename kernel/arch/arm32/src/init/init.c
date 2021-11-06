#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/intr/gtimer.h>
#include <arch/intr/gic.h>

#include <arch/registers.h>
#include <arch/plat/platform.h>

#include <arch/init/hw_init.h>

#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>


void arch_init(void) {
    disable_interrupts();
    disable_fiqs();

    if(hw_init_console()) {
        /* Of course, if we fail to initialize the console, this message may
         * not be visible/transmitted. */
        kpanic("Could not initialize kernel console!");
    }
    kerror(ERR_INFO, "Console Initialized");

    intr_init();
    hw_init_interrupts();
    kerror(ERR_INFO, "Interrupts Initialized");

    hw_init_mm();
}