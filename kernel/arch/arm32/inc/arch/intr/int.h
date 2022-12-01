#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#include <err/panic.h>
#include <intr/types/intr.h>

#define enable_interrupts()  asm volatile("cpsie i")
#define disable_interrupts() asm volatile("cpsid i")
#define interrupt_halt()     asm volatile("wfi")
#define busy_wait()          asm volatile("wfi")
//#define interrupt_halt()     asm volatile("nop")
//#define busy_wait()          asm volatile("nop")

#define enable_fiqs()  asm volatile("cpsie f")
#define disable_fiqs() asm volatile("cpsid f")




#include <arch/intr/gic.h>

static inline int interrupts_enabled() {
    uint32_t tmp;
    asm volatile("mrs %0, cpsr" : "=r" (tmp));
    return !(tmp & 0x80);
}

void intr_set_handler(interrupt_idx_e idx, intr_handler_hand_t *hdlr);

void intr_init(void);

void intr_attach_irqhandler(void (*handler)(void *), void *data);

/**
 * @brief Attach handler for interrupt
 *
 * @param idx Interrupt ID
 * @param hdlr Interrupt handler handle
 */
static inline void arch_interrupt_attach(interrupt_idx_e n, intr_handler_hand_t *hdlr) {
//    idt_add_callback((uint8_t)n, hdlr);
    intr_set_handler(n, hdlr);
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
