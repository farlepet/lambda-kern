#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <types.h>

static inline void enable_interrupts() //!< Enable interrupts
{
	asm volatile("sti");
}

static inline void disable_interrupts() //!< Disable interrupts
{
	asm volatile("cli");
}

static inline void interrupt_halt() //!< Halt until next interrupt
{
	asm volatile("hlt");
}

static inline void busy_wait() //!< Preform lower-power wait
{
	enable_interrupts(); // If interrupts are disables, it will just hang
	asm volatile("rep; nop");
}

int interrupts_enabled(); //!< in intr.asm

#endif