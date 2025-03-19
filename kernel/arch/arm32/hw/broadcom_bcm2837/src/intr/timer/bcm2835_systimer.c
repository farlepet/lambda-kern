#include <errno.h>
#include <string.h>

#include <arch/intr/timer/bcm2835_systimer.h>
#include <arch/intr/bcm2835_intctlr.h>

#include <err/error.h>

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq);
static int _timerdev_attach(void *data, uint8_t idx, void (*callback)(void));

int bcm2835_systimer_create_timerdev(bcm2835_systimer_handle_t *hand, hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->data = (void *)hand;

    dev->setfreq   = _timerdev_setfreq;
    dev->setperiod = NULL; /* TODO */
    dev->attach    = _timerdev_attach;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;

    return 0;
}

static hal_clock_dev_t _dummy_clock = { .freq = 800000 };
int bcm2835_systimer_init(bcm2835_systimer_handle_t *hand, void *base) {
    if(!hand || !base) {
        return -EINVAL;
    }

    memset(hand, 0, sizeof(bcm2835_systimer_handle_t));

    /* TODO: Determine actual source clock */
    hand->src_clock = &_dummy_clock;

    hand->base = (bcm2835_systimer_regmap_t *)base;

    /* "Disable" all timers: */
    hand->reload[0] = 0;
    hand->reload[1] = 0;

    return 0;
}

static void _intr_handler_1(uint32_t __unused int_n, void *data) {
    bcm2835_systimer_handle_t *hand = (bcm2835_systimer_handle_t *)data;
    /* Acknowledge */
    hand->base->status = (1UL << BCM2835_SYSTIMER_STATUS_M1__POS);
    
    if(hand->reload[0]) {
        if(hand->reload[0] == 0xFFFFFFFF) {
            /* One-shot mode */
            hand->reload[0] = 0;
        } else {
            hand->base->compare[1] = hand->base->count_l + hand->reload[0];
        }

        if(hand->callbacks[0]) {
            hand->callbacks[0]();
        }
    }
}

static void _intr_handler_3(uint32_t __unused int_n, void *data) {
    bcm2835_systimer_handle_t *hand = (bcm2835_systimer_handle_t *)data;
    /* Acknowledge */
    hand->base->status = (1UL << BCM2835_SYSTIMER_STATUS_M3__POS);

    if(hand->reload[1]) {
        if(hand->reload[1] == 0xFFFFFFFF) {
            /* One-shot mode */
            hand->reload[1] = 0;
        } else {
            hand->base->compare[3] = hand->base->count_l + hand->reload[1];
        }

        if(hand->callbacks[1]) {
            hand->callbacks[1]();
        }
    }
}

int bcm2835_systimer_int_attach(bcm2835_systimer_handle_t *hand, hal_intctlr_dev_t *intctlr) {
    hal_intctlr_dev_intr_attach(intctlr, BCM2835_IRQ_SYSTIMER_MATCH1, _intr_handler_1, hand);
    hal_intctlr_dev_intr_attach(intctlr, BCM2835_IRQ_SYSTIMER_MATCH3, _intr_handler_3, hand);
    hal_intctlr_dev_intr_enable(intctlr, BCM2835_IRQ_SYSTIMER_MATCH1);
    hal_intctlr_dev_intr_enable(intctlr, BCM2835_IRQ_SYSTIMER_MATCH3);

    return 0;
}

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq) {
    bcm2835_systimer_handle_t *hand = (bcm2835_systimer_handle_t *)data;
    
    if(idx >= 2) {
        return -EINVAL;
    }
    if(freq > hand->src_clock->freq) {
        return -EUNSPEC;
    }

    hand->reload[idx] = (hand->src_clock->freq / freq);
    hand->base->compare[(2*idx)+1] = hand->base->count_l + hand->reload[idx];

    return 0;
}

static int _timerdev_attach(void *data, uint8_t idx, void (*callback)(void)) {
    if(idx >= 2) {
        return -EINVAL;
    }
    
    bcm2835_systimer_handle_t *hand = (bcm2835_systimer_handle_t *)data;

    if(hand->callbacks[idx]) {
        /* Presently only support a single callback per timer */
        return -EUNSPEC;
    }
    
    hand->callbacks[idx] = callback;

    return 0;
}
