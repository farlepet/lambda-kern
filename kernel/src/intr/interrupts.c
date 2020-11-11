#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <types.h>

#if defined(ARCH_X86)
#  include <arch/intr/idt.h>
#  include <arch/intr/pit.h>
#elif defined(ARCH_ARMV7)
#  include <arch/intr/gtimer.h>
#  include <arch/intr/timer/timer_sp804.h>
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

#ifndef ARCH_X86
/* TODO: Move elsewhere */
__hot
static void clk_count() {
	kerneltime++;
	
	for(uint32_t i = 0; i < MAX_TIME_BLOCKS; i++)
		if(time_blocks[i].event)
			if(--time_blocks[i].count == 0x00000000)
				do_time_block_timeup(i);
}
#endif

/* TODO: Move HAL elsewhere */
static hal_timer_dev_t timer;
/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum) {
#ifdef ARCH_X86
	pit_init(quantum);
	pit_create_timerdev(&timer);
	hal_timer_dev_attach(&timer, 0, do_task_switch);
#else
	static timer_sp804_handle_t sp804;
	extern hal_intctlr_dev_t intctlr; /* GIC */

    timer_sp804_init(&sp804, VEXPRESS_A9_PERIPH_TIMER01_BASE);
	timer_sp804_int_attach(&sp804, &intctlr, VEXPRESSA9_INT_TIM01);
	timer_sp804_create_timerdev(&sp804, &timer);
	hal_timer_dev_setfreq(&timer, 0, quantum);
	hal_timer_dev_attach(&timer, 0, do_task_switch);
	hal_timer_dev_setfreq(&timer, 1, 1000);
	hal_timer_dev_attach(&timer, 1, clk_count);
#endif
	kerror(ERR_BOOTINFO, "Timer initialized");
}
