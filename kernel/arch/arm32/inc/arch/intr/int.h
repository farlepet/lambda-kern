#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>


#define enable_interrupts()  asm volatile("cpsie i")
#define disable_interrupts() asm volatile("cpsid i")
#define interrupt_halt()     asm volatile("wfi")
#define busy_wait()          asm volatile("wfi")
//#define interrupt_halt()     asm volatile("nop")
//#define busy_wait()          asm volatile("nop")

#define enable_fiqs()  asm volatile("cpsie f")
#define disable_fiqs() asm volatile("cpsid f")

typedef enum {
    INTR_RESET         = 0,
    INTR_UNDEFINED     = 1,
    INTR_SYSCALL       = 2,
    INTR_PREFETCHABORT = 3,
    INTR_DATAABORT     = 4,
    INTR_HYPTRAP       = 5,
    INTR_IRQ           = 6,
    INTR_FIQ           = 7,
    INTR_MAX           = 8
} interrupt_idx_e;


#include <arch/intr/gic.h>

static inline int interrupts_enabled() {
    uint32_t tmp;
    asm volatile("mrs %0, cpsr" : "=r" (tmp));
    return !(tmp & 0x80);
}

void intr_set_handler(interrupt_idx_e idx, void (*handler)(uint8_t, uintptr_t));

void intr_init(void);

void intr_attach_irqhandler(void (*handler)(void *), void *data);

#endif