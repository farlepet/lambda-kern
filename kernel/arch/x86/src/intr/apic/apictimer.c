#include "hal/timer/timer.h"
#include <err/error.h>
#include <intr/intr.h>
#include <intr/types/intr.h>
#include <time/time.h>

#include <arch/intr/types/intr.h>
#include <arch/intr/apic/apictimer.h>

#include <string.h>

/* NOTE: Divisor = 2 ^ D, except for where D == 7, where Divisor = 1 */
#define DIVISOR_CONV(D) (((D) & 0x03) | (((D) & 0x04) << 1))

static int _timerdev_setfreq(void *, uint8_t, uint32_t);
static int _timerdev_attach(void *, uint8_t, void (*callback)(void));
static void _lapictimer_handler(intr_handler_hand_t *hdlr);

static void (*lapictimer_callback)(void) = NULL;

/* TODO: This should be allocated, as we may need one for every core. */
static intr_handler_hand_t _lapictimer_int_hdlr = {
    .callback = _lapictimer_handler,
    .data     = NULL
};

void apictimer_init(apictimer_handle_t *hand) {
    memset(hand, 0, sizeof(apictimer_handle_t));

    hand->lapic = (apic_lapic_regs_t *)apic_getaddr();

    /* TODO: Calibration via PIT */
    hand->busfreq = 10000000;

    interrupt_attach(INTR_APICTIMER, &_lapictimer_int_hdlr);
}

void apictimer_create_timerdev(apictimer_handle_t *hand, hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->data = hand;

    dev->setfreq   = _timerdev_setfreq;
    dev->setperiod = NULL;
    dev->attach    = _timerdev_attach;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;
}

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq) {
    if(idx >= 1) {
        return -1;
    }
    apictimer_handle_t *hand = (apictimer_handle_t *)data;
    if(freq > (hand->busfreq / 2)) {
        return -1;
    }

    /* Just a static divisor of 2 for now */
    hand->lapic->timer_div_config.value = DIVISOR_CONV(0);

    /* Only periodic for now */
    hand->lapic->lvt_timer.value = INTR_APICTIMER | (LAPIC_LVTENTRY_TIMERMODE_PERIODIC << LAPIC_LVTENTRY_TIMERMODE__POS);

    /** Set the reload value and start the timer */
    hand->lapic->timer_initial_cnt.value = hand->busfreq / freq;

    return 0;
}

static int _timerdev_attach(void __unused *data, uint8_t idx, void (*callback)(void)) {
    if(idx != 0) {
        return -1;
    }

    if(lapictimer_callback) {
        /* Presently only support a single callback */
        return -1;
    }

    lapictimer_callback = callback;

    return 0;
}

static void _lapictimer_handler(intr_handler_hand_t __unused *hdlr) {
    if(lapictimer_callback) {
        lapictimer_callback();
    }
}
