#include <errno.h>
#include <string.h>

#include <err/error.h>

#include <arch/intr/bcm2835_intctlr.h>
#include <arch/intr/int.h>

static int  _intctlr_intr_enable(void *data, uint32_t int_n);
static int  _intctlr_intr_disable(void *data, uint32_t int_n);
static int  _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data);
static void _irqhandle(void *data);

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

    /* Disable all interrupt sources */
    hand->regmap->irq_basic_disable = 0xFF;
    hand->regmap->irq_disable[0] = 0xFFFFFFFF;
    hand->regmap->irq_disable[1] = 0xFFFFFFFF;
    
    intr_attach_irqhandler(_irqhandle, hand);

    return 0;
}

__inline
void _call_handler(bcm2835_intctlr_handle_t *hand, uint8_t int_n) {
    for(uint16_t i = 0; i < BCM2835_INTCTLR_MAX_CALLBACKS; i++) {
        if((hand->callbacks[i].int_n == int_n) &&
           hand->callbacks[i].callback) {
            hand->callbacks[i].callback(int_n, hand->callbacks[i].data);
            /* NOTE: If multiple callbacks for a single interrupt ar desired,
             * the break can simply be removed. */
            break;
        }
    }
}

__hot
static void _irqhandle(void *data) {
    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    /* GPU Interrupts */
    if(hand->regmap->irq_basic_pending & (1UL << BCM2835_INT_GPUIRQ_0)) {
        for(uint8_t i = 0; i < 32; i++) {
            if(hand->regmap->irq_pending[0] & (1UL << i)) {
                _call_handler(hand, i);
            }
        }
    }
    if(hand->regmap->irq_basic_pending & (1UL << BCM2835_INT_GPUIRQ_1)) {
        for(uint8_t i = 0; i < 32; i++) {
            if(hand->regmap->irq_pending[1] & (1UL << i)) {
                _call_handler(hand, 32 + i);
            }
        }
    }
    /* ARM Interrupts */
    for(uint8_t i = 0; i < 8; i++) {
        if(hand->regmap->irq_basic_pending & (1UL << i)) {
            _call_handler(hand, BCM2835_INT_OFFSET + i);
        }
    }
}

static int _intctlr_intr_enable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -EINVAL;
    }

    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    if(int_n < 32) {
        hand->regmap->irq_enable[0] = (1UL << int_n);
    } else if(int_n < 64) {
        hand->regmap->irq_enable[1] = (1UL << (int_n - 32));
    } else if((int_n > BCM2835_INT_OFFSET) &&
              (int_n < (BCM2835_INT_OFFSET + 8))) {
        hand->regmap->irq_basic_enable = (1UL << (int_n - BCM2835_INT_OFFSET));
    } else {
        return -EINVAL;
    }

    return 0;
}

static int _intctlr_intr_disable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -EINVAL;
    }

    bcm2835_intctlr_handle_t *hand = (bcm2835_intctlr_handle_t *)data;

    if(int_n < 32) {
        hand->regmap->irq_disable[0] = (1UL << int_n);
    } else if(int_n < 64) {
        hand->regmap->irq_disable[1] = (1UL << (int_n - 32));
    } else if((int_n > BCM2835_INT_OFFSET) &&
              (int_n < (BCM2835_INT_OFFSET + 8))) {
        hand->regmap->irq_basic_disable = (1UL << (int_n - BCM2835_INT_OFFSET));
    } else {
        return -EINVAL;
    }

    return 0;
}

static int _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data) {
    if (int_n > 255) {
        return -EINVAL;
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
    return -EUNSPEC;
}
