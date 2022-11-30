#ifndef ARCH_INTR_TYPES_INTR_H
#define ARCH_INTR_TYPES_INTR_H

#include <stdint.h>

typedef enum x86_interrupt_idx_enum {
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
 * Registers pushed on the stack via `pusha`
 */
typedef struct x86_pusha_regs_struct {
    uint32_t edi, esi;
    uint32_t ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} x86_pusha_regs_t;

/**
 * Registers pushed on the stack on an interrupt
 */
typedef struct x86_iret_regs_struct {
    uint32_t eip, cs;
    uint32_t eflags;
    uint32_t esp, ds;
} x86_iret_regs_t;

typedef struct x86_intr_handler_hand_data_struct {
    x86_iret_regs_t  *iregs; /**< Pointer to iret regs at interrupt entrypoint */
    x86_pusha_regs_t *pregs; /**< Pointer to pusha regs at interrupt entrypoint */
} arch_intr_handler_hand_data_t;


#endif

