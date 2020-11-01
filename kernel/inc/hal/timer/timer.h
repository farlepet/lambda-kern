#ifndef HAL_TIMER_TIMER_H
#define HAL_TIMER_TIMER_H

#include <types.h>

typedef struct {
    int (*setfreq)(void *data, uint8_t idx, uint32_t freq);         //!< Set timer rate, >= 1Hz
    int (*setperiod)(void *data, uint8_t idx, uint32_t period);     //!< Set timer rate, <= 1Hz
    int (*attach)(void *data, uint8_t idx, void (*callback)(void)); //!< Attach callback (return -1 if full)

    void *data;

#define HAL_TIMERDEV_CAP_VARFREQ  (1UL << 0) //!< Configurable frequency
#define HAL_TIMERDEV_CAP_CALLBACK (1UL << 1) //!< Attachable callback support
    uint32_t cap;
} hal_timer_dev_t;

static inline int hal_timer_dev_setfreq(hal_timer_dev_t *dev, uint8_t idx, uint32_t freq) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_TIMERDEV_CAP_VARFREQ) || !dev->setfreq) return -1;

    return dev->setfreq(dev->data, idx, freq);
}

static inline int hal_timer_dev_setpperiod(hal_timer_dev_t *dev, uint8_t idx, uint32_t period) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_TIMERDEV_CAP_VARFREQ) || !dev->setperiod) return -1;

    return dev->setperiod(dev->data, idx, period);
}

static inline int hal_timer_dev_attach(hal_timer_dev_t *dev, uint8_t idx, void (*callback)(void)) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_TIMERDEV_CAP_CALLBACK) || !dev->attach) return -1;

    return dev->attach(dev->data, idx, callback);
}

#endif
