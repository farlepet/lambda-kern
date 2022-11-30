#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <stdint.h>

#include <types.h>
#include <intr/types/intr.h>

#include <arch/intr/idt.h>

#define enable_interrupts()  asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define interrupt_halt()     asm volatile("hlt")
#define busy_wait()          asm volatile("hlt")

#define INTERRUPT(int_n) __INTERRUPT(int_n) //!< Call interrupt
#define __INTERRUPT(int_n) asm volatile("int $" #int_n)


/**
 * @brief Checks if interrupts are currently enabled
 * 
 * @return int 1 if enabled, else 0.
 */
int interrupts_enabled(void);

/**
 * @brief Attach handler for interrupt
 *
 * @param idx Interrupt ID
 * @param hdlr Interrupt handler handle
 */
static inline void arch_interrupt_attach(interrupt_idx_e n, intr_handler_hand_t *hdlr) {
    idt_add_callback((uint8_t)n, hdlr);
}

/**
 * @brief Setup handler for interrupt, callable from userspace
 *
 * @param idx Interrupt ID
 * @param handler Handler
 */
void arch_set_interrupt_handler_user(interrupt_idx_e idx, void *handler);

extern void call_syscall_int(uint32_t, syscallarg_t *);
/**
 * @brief Call syscall
 *
 * @param scn Syscall number
 * @param args Pointer to arguments list
 */
static inline void arch_call_syscall(uint32_t scn, syscallarg_t *args) {
    call_syscall_int(scn, args);
}

/**
 * @brief Force division by zero, for debugging purposes
 */
static inline void arch_div0(void) {
    asm volatile("mov $0, %%ecx\n"
                 "divl    %%ecx\n"
                 ::: "%eax", "%ecx");
}

#endif

