#ifndef HAL_INTR_INTCTLR_H
#define HAL_INTR_INTCTLR_H

#include <types.h>

typedef struct {
    int (*intr_enable)(void *data, uint32_t int_n);                                                     //!< Enable interrupt
    int (*intr_disable)(void *data, uint32_t int_n);                                                    //!< Disable interrupt
    int (*intr_attach)(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data); //!< Attach interrupt handler

    void *data;

#define HAL_INTCTLRDEV_CAP_INTENDISABLE  (1UL << 0) //!< Interrupt en/disable
#define HAL_INTCTLRDEV_CAP_INTATTACH     (1UL << 1) //!< Interrupt handler attach
    uint32_t cap;
} hal_intctlr_dev_t;

static inline int hal_intctlr_dev_intr_enable(hal_intctlr_dev_t *dev, uint32_t int_n) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_INTCTLRDEV_CAP_INTENDISABLE) || !dev->intr_enable) return -1;

    return dev->intr_enable(dev->data, int_n);
}

static inline int hal_intctlr_dev_intr_disable(hal_intctlr_dev_t *dev, uint32_t int_n) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_INTCTLRDEV_CAP_INTENDISABLE) || !dev->intr_disable) return -1;

    return dev->intr_disable(dev->data, int_n);
}

static inline int hal_intctlr_dev_intr_attach(hal_intctlr_dev_t *dev, uint32_t int_n, void (*callback)(uint32_t, void *), void *data) {
    if(!dev) return -1;
    if(!(dev->cap & HAL_INTCTLRDEV_CAP_INTATTACH) || !dev->intr_attach) return -1;

    return dev->intr_attach(dev->data, int_n, callback, data);
}

#endif
