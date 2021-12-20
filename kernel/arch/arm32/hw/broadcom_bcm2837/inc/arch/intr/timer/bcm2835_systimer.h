#ifndef ARCH_ARM32_INTR_TIMER_BCM2835_SYSTIMER_H
#define ARCH_ARM32_INTR_TIMER_BCM2835_SYSTIMER_H

#include <stdint.h>

#include <hal/timer/timer.h>
#include <hal/intr/int_ctlr.h>
#include <hal/clock/clock.h>

typedef struct {
    volatile uint32_t status;     /*!< Control & status */
#define BCM2835_SYSTIMER_STATUS_M0__POS (0UL) /*!< Timer 0 match */
#define BCM2835_SYSTIMER_STATUS_M1__POS (1UL) /*!< Timer 1 match */
#define BCM2835_SYSTIMER_STATUS_M2__POS (2UL) /*!< Timer 2 match */
#define BCM2835_SYSTIMER_STATUS_M3__POS (3UL) /*!< Timer 3 match */
    volatile uint32_t count_l;    /*!< Lower 32-bits of counter */
    volatile uint32_t count_h;    /*!< Upper 32-bits of counter */
    volatile uint32_t compare[4]; /*!< Four timer comparison values */
} bcm2835_systimer_regmap_t;

typedef struct {
    bcm2835_systimer_regmap_t *base;      /*!< Register map base address */
    hal_clock_dev_t           *src_clock; /*!< Clock source for systimer */

    uint32_t                   reload[2]; /*!< Reload values when timers expire */

    void (*callbacks[2])(void);           /*!< Registered callbacks */
} bcm2835_systimer_handle_t;

int bcm2835_systimer_create_timerdev(bcm2835_systimer_handle_t *hand, hal_timer_dev_t *dev);

int bcm2835_systimer_init(bcm2835_systimer_handle_t *hand, void *base);

int bcm2835_systimer_int_attach(bcm2835_systimer_handle_t *hand, hal_intctlr_dev_t *intctlr);

#endif
