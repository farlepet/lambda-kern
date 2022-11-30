#ifndef INTR_TYPES_INTR_H
#define INTR_TYPES_INTR_H

typedef struct intr_handler_hand_struct intr_handler_hand_t;

#include <arch/intr/types/intr.h>

/**
 * Interrupt handler handle
 */
struct intr_handler_hand_struct {
    /**
     * @brief Pointer to interrupt callback function
     *
     * @param hdlr Pointer to this structure
     */
    void (*callback)(intr_handler_hand_t *hdlr);
    /** Data to be used by interrupt handler, if needed */
    void  *data;
    /** Architecture-specific interrupt handler data */
    arch_intr_handler_hand_data_t arch;
};

#endif

