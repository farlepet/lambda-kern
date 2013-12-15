#include <types.h>

#ifndef INTERRUPT_H
#define INTERRUPT_H

#if defined(ARCH_X86) | defined(ARCH_X86_64)

#define TIMER_INT    32
#define KEYBOARD_INT 33

#else

#define TIMER_INT
#define KEYBOARD_INT

#endif

void interrupts_init();

void set_interrupt(u32 n, void *handler);

void timer_init(u32 quantum);

#endif