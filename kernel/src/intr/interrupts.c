#include <intr/intr.h>
#include <err/error.h>
#include <types.h>

#if defined(ARCH_X86)
#  include <intr/idt.h>
#  include <intr/pit.h>
#  include <intr/int.h>
#endif


/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(uint32_t n, void *handler) {
#if   defined(ARCH_X86)
	set_idt((uint8_t)n, 0x08, 0x8E, handler);
#endif
	kerror(ERR_INFO, "Interrupt vector 0x%02X set", n);
}

/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum) {
#ifdef ARCH_X86
	pit_init(quantum);
#else
	(void)quantum;
#endif
	kerror(ERR_BOOTINFO, "Timer initialized");
}
