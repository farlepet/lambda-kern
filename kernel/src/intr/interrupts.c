#include <lambda/export.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <types.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/intr/idt.h>
#  include <arch/intr/pit.h>
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARM32)
#  include <arch/init/hw_init.h>
#endif

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
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	set_idt((uint8_t)n, 0x08, 0x8E, handler);
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARM32)
	intr_set_handler(n, handler);
#endif
	kerror(ERR_DEBUG, "Interrupt vector 0x%02X set", n);
}
EXPORT_FUNC(set_interrupt);

/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum) {
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	/* TODO: Move to x86 arch */
	static hal_timer_dev_t timer;
	pit_init(quantum);
	pit_create_timerdev(&timer);
	hal_timer_dev_attach(&timer, 0, _task_switch_handler);
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARM32)
	hw_init_timer(quantum);
#else
	(void)quantum;
#endif
	kerror(ERR_INFO, "Timer initialized");
}
