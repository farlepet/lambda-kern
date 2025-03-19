#include <errno.h>
#include <string.h>

#include <arch/intr/timer/timer_sp804.h>

#include <io/output.h>

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq);
static int _timerdev_attach(void *data, uint8_t idx, void (*callback)(void));

int timer_sp804_create_timerdev(timer_sp804_handle_t *hand, hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->data = (void *)hand;

    dev->setfreq   = _timerdev_setfreq;
    dev->setperiod = NULL; /* TODO */
    dev->attach    = _timerdev_attach;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;

    return 0;
}

int timer_sp804_init(timer_sp804_handle_t *hand, void *base) {
    if(!base) {
        return -EINVAL;
    }

    /* TODO: Determine actual source clock */
    hand->srcclk_freq = 800000;

    hand->base = (timer_sp804_regmap_t *)base;

    /* Disable both timers: */
    hand->base->TIM1.CTRL = 0;
    hand->base->TIM2.CTRL = 0;

    return 0;
}

static void _intr_handler(uint32_t __unused int_n, void *data) {
    timer_sp804_handle_t *hand = (timer_sp804_handle_t *)data;

    /* NOTE: With the current design of multithreading, we cannot guarantee that
     * we will always return from the callback. Thus we must clear the interrupt
     * prior to servicing it, and we cannot rely on execution after returning
     * from that call. */
    
    if(hand->base->TIM1.MIS) {
        hand->base->TIM1.INTCLR = 1;
        if(hand->callbacks[0]) {
            hand->callbacks[0]();
        }
    }
    
    if(hand->base->TIM2.MIS) {
        hand->base->TIM2.INTCLR = 1;
        if(hand->callbacks[1]) {
            hand->callbacks[1]();
        }
    }
}

int timer_sp804_int_attach(timer_sp804_handle_t *hand, hal_intctlr_dev_t *intctlr, uint32_t int_n) {
    hal_intctlr_dev_intr_attach(intctlr, int_n, _intr_handler, hand);
    hal_intctlr_dev_intr_enable(intctlr, int_n);

    return 0;
}

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq) {
    timer_sp804_handle_t *hand = (timer_sp804_handle_t *)data;
    
    if(idx >= 2) {
        return -EINVAL;
    }
    if(freq > hand->srcclk_freq) {
        return -EUNSPEC;
    }

    timer_sp804_regmap_tspec_t *timer = (idx == 0) ?
                                        &hand->base->TIM1 :
                                        &hand->base->TIM2;

    timer->CTRL = (timer->CTRL & ~((1UL                             << TIMER_SP804_CTRL_ONESHOT__POS) |
                                   (TIMER_SP804_CTRL_PRESCALE__MASK << TIMER_SP804_CTRL_PRESCALE__POS) |
                                   (0UL                             << TIMER_SP804_CTRL_MODE__POS))) |
                  (1UL << TIMER_SP804_CTRL_SIZE__POS) |
                  (1UL << TIMER_SP804_CTRL_MODE__POS);

    timer->LOAD = hand->srcclk_freq / freq;

    timer->CTRL |= (1UL << TIMER_SP804_CTRL_EN__POS);

    return 0;
}

static int _timerdev_attach(void *data, uint8_t idx, void (*callback)(void)) {
    if(idx >= 2) {
        return -EINVAL;
    }
    
    timer_sp804_handle_t *hand = (timer_sp804_handle_t *)data;

    if(hand->callbacks[idx]) {
        /* Presently only support a single callback per timer */
        return -EUNSPEC;
    }
    
    hand->callbacks[idx] = callback;

    timer_sp804_regmap_tspec_t *timer = (idx == 0) ?
                                        &hand->base->TIM1 :
                                        &hand->base->TIM2;
    
    timer->CTRL |= (1UL << TIMER_SP804_CTRL_INTEN__POS);

    return 0;
}
