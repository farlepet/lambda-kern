#include <errno.h>
#include <string.h>

#include <arch/intr/apic/apictimer.h>

/* NOTE: Divisor = 2 ^ D, except for where D == 7, where Divisor = 1 */
#define DIVISOR_CONV(D) (((D) & 0x03) | (((D) & 0x04) << 1))

static int _timerdev_setfreq(void *, uint8_t, uint32_t);

void apictimer_init(apictimer_handle_t *hand) {
    memset(hand, 0, sizeof(apictimer_handle_t));

    hand->lapic = (apic_lapic_regs_t *)apic_getaddr();
    
    /* TODO: Calibration via PIT */
}

void apictimer_create_timerdev(apictimer_handle_t *hand, hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->data = hand;

    dev->setfreq   = _timerdev_setfreq;
    dev->setperiod = NULL;
    dev->attach    = NULL;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ;
}

static int _timerdev_setfreq(void *data, uint8_t idx, uint32_t freq) {
    if(idx >= 1) {
        return -EINVAL;
    }
    apictimer_handle_t *hand = (apictimer_handle_t *)data;
    if(freq > (hand->busfreq / 2)) {
        return -EINVAL;
    }

    /* Just a static divisor of 2 for now */
    hand->lapic->timer_div_config.value = DIVISOR_CONV(0);

    /* Only periodic for now */
    hand->lapic->lvt_timer.value = ((hand->lapic->lvt_timer.value &
                                     ~(APIC_LVTENTRY_TIMERMODE__MASK  << APIC_LVTENTRY_TIMERMODE__POS)) |
                                    (APIC_LVTENTRY_TIMERMODE_PERIODIC << APIC_LVTENTRY_TIMERMODE__POS));

    /** Set the reload value and start the timer */
    hand->lapic->timer_initial_cnt.value = hand->busfreq / freq;

    return 0;
}
