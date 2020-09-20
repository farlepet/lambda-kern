#include <arch/intr/int.h>

#include <types.h>

#ifndef INTERRUPT_H
#define INTERRUPT_H

#if defined(ARCH_X86)

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

#else

typedef enum {
    INT_RESET         = 0,
    INT_UNDEFINED     = 1,
    INT_SYSCALL       = 2,
    INT_PREFETCHABORT = 3,
    INT_DATAABORT     = 4,
    INT_HYPTRAP       = 5,
    INT_IRQ           = 6,
    INT_FIQ           = 7
} interrupt_idx_e;

#endif

/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(interrupt_idx_e n, void *handler);


/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum);

#endif
