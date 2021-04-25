#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <types.h>

#define enable_interrupts()  asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define interrupt_halt()     asm volatile("hlt")
#define busy_wait()          asm volatile("hlt")

#define INTERRUPT(int_n) __INTERRUPT(int_n) //!< Call interrupt
#define __INTERRUPT(int_n) asm volatile("int $" #int_n)

typedef enum {
    INT_TIMER        = 32,
    INT_KEYBOARD     = 33,
    INT_SERIALA      = 35,
    INT_SERIALB      = 36,
    INT_PARALLELB    = 37,
    INT_FLOPPY       = 38,
    INT_PARALLELA    = 39,
    INT_RTC          = 40,
    INT_IRQ9         = 41,
    INT_IRQ10        = 42,
    INT_IRQ11        = 43,
    INT_PS2          = 44,
    INT_COPROCESSOR  = 45,
    INT_ATAPRIMARY   = 46,
    INT_ATASECONDARY = 47,
    INT_SCHED        = 64,
    INT_SYSCALL      = 255
} interrupt_idx_e;

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
