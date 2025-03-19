#include <errno.h>
#include <string.h>

#include <err/error.h>

#include <arch/intr/gic.h>
#include <arch/intr/int.h>

static int  _intctlr_intr_enable(void *data, uint32_t int_n);
static int  _intctlr_intr_disable(void *data, uint32_t int_n);
static int  _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data);
static void _irqhandle(void *data);

int armv7_gic_create_intctlrdev(armv7_gic_handle_t *hand, hal_intctlr_dev_t *intctlrdev) {
    memset(intctlrdev, 0, sizeof(hal_intctlr_dev_t));

    intctlrdev->data = (void *)hand;

    intctlrdev->intr_enable  = _intctlr_intr_enable;
    intctlrdev->intr_disable = _intctlr_intr_disable;
    intctlrdev->intr_attach  = _intctlr_intr_attach;

    intctlrdev->cap = HAL_INTCTLRDEV_CAP_INTENDISABLE | HAL_INTCTLRDEV_CAP_INTATTACH;

    return 0;
}

int armv7_gic_init(armv7_gic_handle_t *hand, volatile void *icc_base, volatile void *dcu_base) {
    hand->icc = (armv7_gic_icc_regmap_t *)icc_base;
    hand->dcu = (armv7_gic_dcu_regmap_t *)dcu_base;

    hand->icc->PMR  = 0x00FF; /* Enable full range of interrupt priorities */
    hand->icc->CTLR = (1UL << ARM32_GIC_ICC_CTLR_GRP1ENABLE__POS);

    hand->dcu->CTLR |= (1UL << ARM32_GIC_DCU_CTLR_SECUREEN__POS) |
                       (1UL << ARM32_GIC_DCU_CTLR_NONSECUREEN__POS);

    for(int i = 0; i < ARM32_GIC_MAX_CALLBACKS; i++) {
        hand->callbacks[i].int_n = 0xFFFFFFFFUL;
    }
    
    intr_attach_irqhandler(_irqhandle, hand);

    return 0;
}

__hot
static void _irqhandle(void *data) {
    armv7_gic_handle_t *hand = (armv7_gic_handle_t *)data;

    /* Read and acknowledge interrupt ID */
    uint32_t intr = hand->icc->IAR & 0x00FFFFFF;

    /* Clear interrupt */
    hand->icc->EOIR = intr;

    for(uint16_t i = 0; i < ARM32_GIC_MAX_CALLBACKS; i++) {
        if(hand->callbacks[i].int_n == intr &&
           hand->callbacks[i].callback) {
            hand->callbacks[i].callback(intr, hand->callbacks[i].data);
            /* NOTE: If multiple callbacks for a single interrupt ar desired,
             * the break can simply be removed. */
            break;
        }
    }

    /* NOTE: With the current design of multithreading, we cannot guarantee that
     * we will reach this point every time. Thus we must clear the interrupt
     * prior to servicing it, and we cannot rely on execution after returning
     * from that call. */
}

static int _intctlr_intr_enable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -EINVAL;
    }

    armv7_gic_handle_t *hand = (armv7_gic_handle_t *)data;

    /* Enable interrupt */
    hand->dcu->ISENABLER[int_n >> 5] |= 1UL << (int_n & 31);

    /* Forward to CPU 0, if multi-CPU */
    hand->dcu->ITARGETSR[(int_n >> 2)] |= 1UL << ((int_n & 3) << 3);

    return 0;
}

static int _intctlr_intr_disable(void *data, uint32_t int_n) {
    if (int_n > 255) {
        return -EINVAL;
    }

    armv7_gic_handle_t *hand = (armv7_gic_handle_t *)data;

    /* Disable interrupt */
    hand->dcu->ICENABLER[int_n >> 5] |= 1UL << (int_n & 31);

    return 0;
}

static int _intctlr_intr_attach(void *data, uint32_t int_n, void (*callback)(uint32_t, void *), void *int_data) {
    if (int_n > 255) {
        return -EINVAL;
    }

    armv7_gic_handle_t *hand = (armv7_gic_handle_t *)data;

    for(int i = 0; i < ARM32_GIC_MAX_CALLBACKS; i++) {
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
