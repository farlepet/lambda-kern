#include <string.h>

#include <err/error.h>

#include <arch/intr/bcm2835_intctlr.h>

static int _intctlr_intr_enable(void *data, uint32_t int_n);
static int _intctlr_intr_disable(void *data, uint32_t int_n);
static int _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data);

int bcm2835_intctlr_create_intctlrdev(bcm2835_intctlr_handle_t *hand, hal_intctlr_dev_t *intctlrdev) {
    memset(intctlrdev, 0, sizeof(hal_intctlr_dev_t));

    intctlrdev->data = (void *)hand;

    intctlrdev->intr_enable  = _intctlr_intr_enable;
    intctlrdev->intr_disable = _intctlr_intr_disable;
    intctlrdev->intr_attach  = _intctlr_intr_attach;

    intctlrdev->cap = HAL_INTCTLRDEV_CAP_INTENDISABLE | HAL_INTCTLRDEV_CAP_INTATTACH;

    return 0;
}

int bcm2835_intctlr_init(bcm2835_intctlr_handle_t *hand, volatile void *base) {
    hand->regmap = (bcm2835_intctlr_regmap_t *)base;

    for(int i = 0; i < BCM2835_INTCTLR_MAX_CALLBACKS; i++) {
        hand->callbacks[i].int_n = 0xFFFFFFFFUL;
    }

    return 0;
}

__hot
int bcm2835_intctlr_irqhandle(bcm2835_intctlr_handle_t *hand) {
    /* TODO */
    (void)hand;
#if 0
    int ret = 1;
    /* Read and acknowledge interrupt ID */
    uint32_t intr = hand->icc->IAR & 0x00FFFFFF;

    /* Clear interrupt */
    hand->icc->EOIR = intr;

    for(uint16_t i = 0; i < ARM32_GIC_MAX_CALLBACKS; i++) {
        if(hand->callbacks[i].int_n == intr &&
           hand->callbacks[i].callback) {
            hand->callbacks[i].callback(intr, hand->callbacks[i].data);
            ret = 0;
            /* NOTE: If multiple callbacks for a single interrupt ar desired,
             * the break can simply be removed. */
            break;
        }
    }

    /* NOTE: With the current design of multithreading, we cannot guarantee that
     * we will reach this point every time. Thus we must clear the interrupt
     * prior to servicing it, and we cannot rely on execution after returning
     * from that call. */

    return ret;
#endif

    return 0;
}

static int _intctlr_intr_enable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -1;
    }

    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    /* TODO */
    (void)hand;

    return 0;
}

static int _intctlr_intr_disable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -1;
    }

    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    /* TODO */
    (void)hand;

    return 0;
}

static int _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data) {
    if (int_n > 255) {
        return -1;
    }

    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    for(int i = 0; i < BCM2835_INTCTLR_MAX_CALLBACKS; i++) {
        if(hand->callbacks[i].int_n == 0xFFFFFFFFUL) {
            hand->callbacks[i].int_n    = int_n;
            hand->callbacks[i].data     = int_data;
            hand->callbacks[i].callback = callback;
            return 0;
        }
    }

    /* No free slots */
    return -1;
}
