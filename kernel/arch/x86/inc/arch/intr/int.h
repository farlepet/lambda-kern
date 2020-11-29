#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <types.h>

#define enable_interrupts()  asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define interrupt_halt()     asm volatile("hlt")
#define busy_wait()          asm volatile("hlt")

#define INTERRUPT(int_n) __INTERRUPT(int_n) //!< Call interrupt
#define __INTERRUPT(int_n) asm volatile("int $" #int_n)

/**
 * @brief Checks if interrupts are currently enabled
 * 
 * @return int 1 if enabled, else 0.
 */
int interrupts_enabled();


typedef struct pusha_regs {
	uint32_t edi, esi;
	uint32_t ebp, esp;
	uint32_t ebx, edx, ecx, eax;
} arch_pusha_regs_t;

typedef struct iret_regs {
	uint32_t eip, cs;
	uint32_t eflags;
	uint32_t esp, ds;
} arch_iret_regs_t;

#endif
