#include <arch/init/init.h>
#include <arch/intr/gtimer.h>

void arch_init(struct multiboot_header *mboot_head) {
    (void)mboot_head;

    /* 1 kHz / 8 = 125Hz interrupt */
    armv7_gtimer_init(1000);
}