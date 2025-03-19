#include <errno.h>
#include <string.h>

#include <arch/intr/gtimer.h>

static void (*gtimer_callback)(void) = NULL;

static int timerdev_setfreq(void *data, uint8_t idx, uint32_t freq);
static int timerdev_attach(void *data, uint8_t idx, void (*callback)(void));

void armv7_gtimer_create_timerdev(hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->setfreq   = timerdev_setfreq;
    dev->setperiod = NULL;
    dev->attach    = timerdev_attach;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;
}

int armv7_gtimer_init(uint32_t freq) {
    uint32_t tmp;
    
    __WRITE_CNTFRQ(freq);

    //__READ_CNTKCTL(tmp);
    tmp  = (1UL << 1) | /* Enable PL0 access to Vitual timer */
           (1UL << 2) | /* Enable event */
           (8UL << 4) | /* Trigger event on 8th bit of CNTVCT */
           (1UL << 8);  /* Enable PL0 access to Vitual timer */
    __WRITE_CNTKCTL(tmp);

    //__READ_CNTV_CTL(tmp);
    tmp = (1UL << 0); /* Enable virtual timer */
    __WRITE_CNTV_CTL(tmp);

    /* Clear CompareValue register */
    tmp = 0;
    __WRITE_CNTV_CVAL(tmp, tmp);

    return 0;
}

static int timerdev_setfreq(void __unused *data, uint8_t idx, uint32_t freq) {
    if(idx != 0) {
        return -EINVAL;
    }
    
    __WRITE_CNTFRQ(freq);

    return 0;
}

static int timerdev_attach(void __unused *data, uint8_t idx, void (*callback)(void)) {
    if(idx != 0) {
        return -EINVAL;
    }
    
    if(gtimer_callback) {
        /* Presently only support a single callback */
        return -EUNSPEC;
    }

    gtimer_callback = callback;

    return 0;
}
