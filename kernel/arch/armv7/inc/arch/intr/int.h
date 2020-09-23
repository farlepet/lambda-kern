#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#include <intr/intr.h>

#include <arch/registers.h>

#define enable_interrupts()  asm volatile("cpsie i")
#define disable_interrupts() asm volatile("cpsid i")
#define interrupt_halt()     asm volatile("wfi")
#define busy_wait()          asm volatile("wfi")

static inline int interrupts_enabled() {
    uint32_t tmp;
    asm volatile("mrs r0, cpsr;\n\
                  mov r0, %0": "=r" (tmp));
    return !(tmp & 0x80);
}

void intr_set_handler(interrupt_idx_e idx, void *ptr);

void intr_init(void);

#endif