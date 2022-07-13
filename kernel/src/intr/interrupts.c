#include <lambda/export.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <types.h>

#include <arch/init/hw_init.h>
#include <arch/init/init.h>
#include <arch/intr/int.h>
#include <arch/proc/tasking.h>


/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(interrupt_idx_e n, void *handler) {
	arch_set_interrupt_handler(n, handler);
	kerror(ERR_DEBUG, "Interrupt vector 0x%02X set", n);
}
EXPORT_FUNC(set_interrupt);

/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum) {
	hw_init_timer(quantum);
	
	kerror(ERR_INFO, "Timer initialized");
}
