#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#include <intr/intr.h>

#include <arch/registers.h>
#include <arch/intr/gic.h>

#define enable_interrupts()  asm volatile("cpsie i")
#define disable_interrupts() asm volatile("cpsid i")
#define interrupt_halt()     asm volatile("wfi")
#define busy_wait()          asm volatile("wfi")

#define enable_fiqs()  asm volatile("cpsie f")
#define disable_fiqs() asm volatile("cpsid f")

static inline int interrupts_enabled() {
    uint32_t tmp;
    asm volatile("mrs %0, cpsr" : "=r" (tmp));
    return !(tmp & 0x80);
}

void intr_set_handler(interrupt_idx_e idx, void (*handler)(uint8_t, uintptr_t));

void intr_init(void);

void intr_attach_gic(armv7_gic_handle_t *hand);

#endif