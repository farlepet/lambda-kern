#ifndef ARCH_INTR_APIC_APICTIMER_H
#define ARCH_INTR_APIC_APICTIMER_H

#include <arch/intr/apic/apic.h>

#include <hal/timer/timer.h>

typedef struct {
    apic_lapic_regs_t *lapic;   /*!< Pointer to local APIC registers */

    uint32_t           busfreq; /*!< Calculated CPU BUS frequency */
} apictimer_handle_t;

/**
 * @brief Initialize local APIC timer
 * 
 * @param hand Timer handle
 */
void apictimer_init(apictimer_handle_t *hand);

/**
 * @brief Create timer device from local APIC timer handle
 * 
 * @param hand Timer handle
 * @param dev Timer device
 */
void apictimer_create_timerdev(apictimer_handle_t *hand, hal_timer_dev_t *dev);

#endif
