#include <intr/intr.h>
#include <err/error.h>
#include <types.h>

#if defined(ARCH_X86)
#  include <arch/intr/idt.h>
#  include <arch/intr/pit.h>
#elif defined(ARCH_ARMV7)
#  include <arch/intr/gtimer.h>
#endif

#include <arch/intr/int.h>
#include <arch/proc/tasking.h>


/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(interrupt_idx_e n, void *handler) {
#if   defined(ARCH_X86)
	set_idt((uint8_t)n, 0x08, 0x8E, handler);
#elif defined(ARCH_ARMV7)
	intr_set_handler(n, handler);
#endif
	kerror(ERR_INFO, "Interrupt vector 0x%02X set", n);
}

/* TODO: Move HAL elsewhere */
static hal_timer_dev_t pit;
/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum) {
#ifdef ARCH_X86
	pit_init(quantum);
	pit_create_timerdev(&pit);
	hal_timer_dev_attach(&pit, do_task_switch);
#else
    armv7_gtimer_init(quantum);
	armv7_gtimer_create_timerdev(&pit);
	hal_timer_dev_attach(&pit, do_task_switch);
#endif
	kerror(ERR_BOOTINFO, "Timer initialized");
}
