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

/** Data stored on stack by interrupt handlers */
typedef struct arm32_intr_frame_struct {
    uint32_t _padding; /**< Padding for 8-byte stack alignment */
    uint32_t cpsr;     /**< USR Current Program Status Register */
    uint32_t r[13];    /**< USR R0-R12 */
    uint32_t lr;       /**< Link Register */
} arm32_intr_frame_t;

typedef struct arm32_intr_handler_hand_data_struct {
    uint32_t  int_n;           /**< Interrupt number */
    arm32_intr_frame_t *frame; /**< Saved interrupts on interrupt */
} arch_intr_handler_hand_data_t;

#endif

