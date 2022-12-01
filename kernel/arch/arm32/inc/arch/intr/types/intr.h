#ifndef ARCH_INTR_TYPES_INTR_H
#define ARCH_INTR_TYPES_INTR_H

typedef enum arm32_interrupt_idx_enum {
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

typedef struct arm32_intr_handler_hand_data_struct {
    uint32_t  int_n; /**< Interrupt number */
    uintptr_t lr;    /**< Link register */
    /* @todo */
} arch_intr_handler_hand_data_t;

#endif

