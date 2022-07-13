#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#include <err/panic.h>

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

/**
 * @brief Setup handler for interrupt
 *
 * @param idx Interrupt ID
 * @param handler Handler
 */
static inline void arch_set_interrupt_handler(interrupt_idx_e idx, void *handler) {
    /* @todo Allow using regular functions with a standard interface as a handler */
    intr_set_handler(idx, (void (*)(uint8_t, uintptr_t))handler);
}

/*, "%ecx"
 * @brief Setup handler for interrupt, callable from userspace
 *
 * @note Only applicable to INTR_SYSCALL for now
 *
 * @param idx Interrupt ID
 * @param handler Handler
 */
static inline void arch_set_interrupt_handler_user(interrupt_idx_e idx, void *handler) {
    /* @todo Allow using regular functions with a standard interface as a handler */
    if(idx != INTR_SYSCALL) {
        kpanic("Attempt to register non-syscall interrupt handler (%d) as user!", idx);
    }
    intr_set_handler(idx, (void (*)(uint8_t, uintptr_t))handler);
}

/**
 * @brief Call syscall
 *
 * @param scn Syscall number
 * @param args Pointer to arguments list
 */
static inline void arch_call_syscall(uint32_t scn, syscallarg_t *args) {
    (void)scn;
    (void)args;
    /* @todo */
    asm volatile("swi #1");
}

/**
 * @brief Force division by zero, for debugging purposes
 */
static inline void arch_div0(void) {
    /* @todo */
#if 0
    asm volatile("mov  r0, #0    \n"
                 "sdiv r0, r0, r0\n"
				 ::: "%r0");
#endif
}

#endif
