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

void intr_set_handler(interrupt_idx_e idx, void *ptr);

void intr_init(void);

void intr_attach_gic(armv7_gic_handle_t *hand);

extern uint32_t intr_stack_end;

#define __SWI_BEGIN \
    uint32_t lr; \
    asm volatile("mov sp, %0" :: "r" (&intr_stack_end)); \
    asm volatile("push {lr}"); \
    asm volatile("push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}"); \
    asm volatile("mov %[ps], lr" : [ps]"=r" (lr));

#define __INTR_BEGIN \
    uint32_t lr; \
    asm volatile("mov sp, %0" :: "r" (&intr_stack_end)); \
    asm volatile("sub lr, lr, #4"); \
    asm volatile("push {lr}"); \
    asm volatile("push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}"); \
    asm volatile("mov %[ps], lr" : [ps]"=r" (lr));

#define __INTR_END \
    asm volatile("pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}"); \
    asm volatile("ldm sp!, {pc}^");

#endif