#include <types.h>
#include <intr/int.h>

#ifndef INTERRUPT_H
#define INTERRUPT_H

#if defined(ARCH_X86)

#  define TIMER_INT    32
#  define KEYBOARD_INT 33
#  define SERIALA_INT  35
#  define SERIALB_INT  36

#  define SCHED_INT    64

#else

#  define TIMER_INT    0
#  define KEYBOARD_INT 0
#  define SERIALA_INT  0
#  define SERIALB_INT  0

#  define SCHED_INT    0

#endif

/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(uint32_t n, void *handler);


/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(uint32_t quantum);

#endif
