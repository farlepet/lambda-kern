#ifndef ARCH_ARM32_INTR_BCM2835_INTCTLR_H
#define ARCH_ARM32_INTR_BCM2835_INTCTLR_H

#include <stdint.h>

#include <hal/intr/int_ctlr.h>

typedef struct {
    uint32_t irq_basic_pending;
    uint32_t irq_pending[2];
    uint32_t fiq_ctrl;
    uint32_t irq_enable[2];
    uint32_t irq_basic_enable;
    uint32_t irq_disable[2];
    uint32_t irq_basic_disable;
} bcm2835_intctlr_regmap_t;

typedef enum {
    BCM2835_IRQ_SYSTIMER_MATCH1 =  1,
    BCM2835_IRQ_SYSTIMER_MATCH3 =  3,
    BCM2835_IRQ_USB_CONTROLLER  =  9,
    BCM2835_IRQ_AUXILIARY       = 29,
    BCM2835_IRQ_I2C_SPI_SLAVE   = 43,
    BCM2835_IRQ_PWA0            = 45,
    BCM2835_IRQ_PWA1            = 45,
    BCM2835_IRQ_SMI             = 48,
    BCM2835_IRQ_GPIO_0          = 49,
    BCM2835_IRQ_GPIO_1          = 50,
    BCM2835_IRQ_GPIO_2          = 51,
    BCM2835_IRQ_GPIO_3          = 52,
    BCM2835_IRQ_I2C             = 53,
    BCM2835_IRQ_SPI             = 54,
    BCM2835_IRQ_PCM             = 55,
    BCM2835_IRQ_UART            = 57,
} bcm2835_irq_e;

typedef enum {
    BCM2835_INT_TIMER         = 0,
    BCM2835_INT_MAILBOX       = 1,
    BCM2835_INT_DOORBELL0     = 2,
    BCM2835_INT_DOORBELL1     = 3,
    BCM2835_INT_GPU0_HALT     = 4,
    BCM2835_INT_GPU1_HALT     = 5,
    BCM2835_INT_ILL_ACCESS_T1 = 6,
    BCM2835_INT_ILL_ACCESS_T2 = 7,
    BCM2835_INT_GPUIRQ_0      = 8,
    BCM2835_INT_GPUIRQ_1      = 9,

    BCM2835_INT_OFFSET        = 128 /*!< Add this offset when registering one of these interrupts */
} bcm2835_int_e;

typedef struct {
    uint32_t int_n;
    void *data;

    void (*callback)(uint32_t, void *);
} bcm2835_intctlr_callback_t;

typedef struct {
    bcm2835_intctlr_regmap_t *regmap;

#define BCM2835_INTCTLR_MAX_CALLBACKS (16)
    bcm2835_intctlr_callback_t callbacks[BCM2835_INTCTLR_MAX_CALLBACKS];    
} bcm2835_intctlr_handle_t;

int bcm2835_intctlr_create_intctlrdev(bcm2835_intctlr_handle_t *hand, hal_intctlr_dev_t *intctlrdev);

int bcm2835_intctlr_init(bcm2835_intctlr_handle_t *hand, volatile void *base);

int bcm2835_intctlr_irqhandle(bcm2835_intctlr_handle_t *hand);

#endif
