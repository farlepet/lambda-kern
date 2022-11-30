#ifndef ARCH_INTR_SYSCALL_H
#define ARCH_INTR_SYSCALL_H

#include <intr/types/intr.h>

/**
 * @brief Architecture-specific handler for the syscall interrupt, calls syscall_service
 */
void arch_syscall_interrupt(intr_handler_hand_t *hdlr);

#endif

