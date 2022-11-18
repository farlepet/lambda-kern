#ifndef INT_FUNCS_H
#define INT_FUNCS_H

#include <types.h>

#include <arch/intr/idt.h>

#define enable_interrupts()  asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define interrupt_halt()     asm volatile("hlt")
#define busy_wait()          asm volatile("hlt")

#define INTERRUPT(int_n) __INTERRUPT(int_n) //!< Call interrupt
#define __INTERRUPT(int_n) asm volatile("int $" #int_n)

typedef enum {
    INTR_TIMER        = 32,
    INTR_KEYBOARD     = 33,
    INTR_SERIALA      = 35,
    INTR_SERIALB      = 36,
    INTR_PARALLELB    = 37,
    INTR_FLOPPY       = 38,
    INTR_PARALLELA    = 39,
    INTR_RTC          = 40,
    INTR_IRQ9         = 41,
    INTR_IRQ10        = 42,
    INTR_IRQ11        = 43,
    INTR_PS2          = 44,
    INTR_COPROCESSOR  = 45,
    INTR_ATAPRIMARY   = 46,
    INTR_ATASECONDARY = 47,
    INTR_SCHED        = 64,
    INTR_SYSCALL      = 255
} interrupt_idx_e;

/**
 * @brief Checks if interrupts are currently enabled
 * 
 * @return int 1 if enabled, else 0.
 */
int interrupts_enabled(void);


typedef struct pusha_regs {
    uint32_t edi, esi;
    uint32_t ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} arch_pusha_regs_t;

typedef struct iret_regs {
    uint32_t eip, cs;
    uint32_t eflags;
    uint32_t esp, ds;
} arch_iret_regs_t;

/**
 * @brief Setup handler for interrupt
 *
 * @param idx Interrupt ID
 * @param handler Handler
 */
static inline void arch_set_interrupt_handler(interrupt_idx_e idx, void *handler) {
    /* @todo Allow using regular functions with a standard interface as a handler */
    set_idt((uint8_t)idx, 0x08, IDT_ATTR(1, 0, 0, int32), handler);
}

/**
 * @brief Setup handler for interrupt, callable from userspace
 *
 * @param idx Interrupt ID
 * @param handler Handler
 */
static inline void arch_set_interrupt_handler_user(interrupt_idx_e idx, void *handler) {
    /* @todo Allow using regular functions with a standard interface as a handler */
    set_idt((uint8_t)idx, 0x08, IDT_ATTR(1, 3, 0, int32), handler);
}

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
