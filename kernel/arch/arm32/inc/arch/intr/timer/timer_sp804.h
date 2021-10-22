#ifndef ARCH_ARM32_INTR_TIMER_TIMER_SP804_H
#define ARCH_ARM32_INTR_TIMER_TIMER_SP804_H

#include <stdint.h>

#include <hal/timer/timer.h>
#include <hal/intr/int_ctlr.h>

#include <arch/plat/platform.h>

typedef struct {
    volatile uint32_t LOAD;   /*!< Load */
    volatile uint32_t VALUE;  /*!< Current value */
#define TIMER_SP804_CTRL_ONESHOT__POS   (   0UL) /*!< One-shot mode enable */
#define TIMER_SP804_CTRL_SIZE__POS      (   1UL) /*!< Counter size (0: 16-bit, 1: 32-bit) */
#define TIMER_SP804_CTRL_PRESCALE__POS  (   2UL) /*!< Prescale value (0: /1, 1: /16, 2: /256, 3: UNDEFINED) */
#define TIMER_SP804_CTRL_PRESCALE__MASK (0x03UL)
#define TIMER_SP804_CTRL_INTEN__POS     (   5UL) /*!< Enable interrupt */
#define TIMER_SP804_CTRL_MODE__POS      (   6UL) /*!< Timer mode (0: free-running, 1: periodic) */
#define TIMER_SP804_CTRL_EN__POS        (   7UL) /*!< Enable timer */
    volatile uint32_t CTRL;   /*!< Control */
    volatile uint32_t INTCLR; /*!< Interrupt clear */
    volatile uint32_t RIS;    /*!< Raw interrupt status */
    volatile uint32_t MIS;    /*!< Masked interrupt status */
    volatile uint32_t BGLOAD; /*!< Background load */
    const uint32_t    __reserved;
} timer_sp804_regmap_tspec_t;

typedef struct {
    timer_sp804_regmap_tspec_t TIM1;      /*!< Timer 1 */
    timer_sp804_regmap_tspec_t TIM2;      /*!< Timer 2 */
    const uint32_t             __reserved0[944];
    volatile uint32_t          ITCR;      /*!< Integration Test Control Register */
    volatile uint32_t          ITOP;      /*!< Integration Test Output Set Register */
    const uint32_t             __reserved1[54];
    volatile uint32_t          PERIPHID0;
    volatile uint32_t          PERIPHID1;
    volatile uint32_t          PERIPHID2;
    volatile uint32_t          PERIPHID3;
    volatile uint32_t          PCELLID0;
    volatile uint32_t          PCELLID1;
    volatile uint32_t          PCELLID2;
    volatile uint32_t          PCELLID3;
} timer_sp804_regmap_t;

typedef struct {
    timer_sp804_regmap_t *base;        /*!< Register map base address */
    uint32_t              srcclk_freq; /*!< Frequency of timer's clock source */

    void (*callbacks[2])(void);        /*!< Registered callbacks */
} timer_sp804_handle_t;

int timer_sp804_create_timerdev(timer_sp804_handle_t *hand, hal_timer_dev_t *dev);

int timer_sp804_init(timer_sp804_handle_t *hand, void *base);

int timer_sp804_int_attach(timer_sp804_handle_t *hand, hal_intctlr_dev_t *intctlr, uint32_t int_n);

#endif
