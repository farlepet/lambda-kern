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

#define INTERRUPT(int_n) __INTERRUPT(int_n) //!< Call interrupt
#define __INTERRUPT(int_n) asm volatile("int $" #int_n)

/**
 * @brief Checks if interrupts are currently enabled
 * 
 * @return int 1 if enabled, else 0.
 */
int interrupts_enabled();


struct pusha_regs {
	uint32_t edi, esi;
	uint32_t ebp, esp;
	uint32_t ebx, edx, ecx, eax;
} __packed;

struct iret_regs {
	uint32_t eip, cs;
	uint32_t eflags;
	uint32_t esp, ds;
} __packed;

#endif