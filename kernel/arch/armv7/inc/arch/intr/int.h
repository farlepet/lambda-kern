#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#define enable_interrupts()  asm volatile("cpsie if")
#define disable_interrupts() asm volatile("cpsid if")
#define interrupt_halt()     asm volatile("wfi")
#define busy_wait()          asm volatile("wfi")

static inline int interrupts_enabled() {
    uint32_t tmp;
    asm volatile("mrs r0, cpsr;\n\
                  mov r0, %0": "=r" (tmp));
    return tmp;
}

#endif