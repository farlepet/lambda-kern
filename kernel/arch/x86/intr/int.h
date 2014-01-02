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
	asm volatile("hlt");
}

int interrupts_enabled(); //!< in intr.asm

void run_usr(void *loc) __noreturn; //!< Jump to usermode code

#endif