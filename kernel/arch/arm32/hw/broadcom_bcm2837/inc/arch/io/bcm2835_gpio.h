#ifndef ARCH_ARM32_INTR_BCM2835_GPIO_H
#define ARCH_ARM32_INTR_BCM2835_GPIO_H

#include <stdint.h>

#include <hal/intr/int_ctlr.h>

#define GPIOPIN_DEBUG_0 (17)
#define GPIOPIN_DEBUG_1 ( 5)
#define GPIOPIN_DEBUG_2 ( 6)
#define GPIOPIN_DEBUG_3 (13)
#define GPIOPIN_DEBUG_4 (19)

typedef volatile struct {
#define BCM2835_GPIO_FUNCTIONSELECT__MASK  (0x07)
#define BCM2835_GPIO_FUNCTIONSELECT_INPUT  (0x00)
#define BCM2835_GPIO_FUNCTIONSELECT_OUTPUT (0x01)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT0   (0x04)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT1   (0x05)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT2   (0x06)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT3   (0x07)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT4   (0x03)
#define BCM2835_GPIO_FUNCTIONSELECT_ALT5   (0x02)
    uint32_t function_select[6];
    uint32_t reserved_00;
    uint32_t output_set[2];
    uint32_t reserved_01;
    uint32_t output_clear[2];
    uint32_t reserved_02;
    uint32_t pin_level[2];
    uint32_t reserved_03;
    uint32_t event_detect_status[2];
    uint32_t reserved_04;
    uint32_t rising_edge_detect_en[2];
    uint32_t reserved_05;
    uint32_t falling_edge_detect_en[2];
    uint32_t reserved_06;
    uint32_t high_detect_en[2];
    uint32_t reserved_07;
    uint32_t low_detect_en[2];
    uint32_t reserved_08;
    uint32_t async_rising_edge_detect[2];
    uint32_t reserved_09;
    uint32_t async_falling_edge_detect[2];
    uint32_t reserved_10;
#define BCM2835_GPIO_PULL_NONE     (0)
#define BCM2835_GPIO_PULL_PULLDOWN (1)
#define BCM2835_GPIO_PULL_PULLUP   (2)
    uint32_t pull;
    uint32_t pull_clock[2];
    uint32_t reserved_11[4];
    uint32_t test;
} bcm2835_gpio_regmap_t;

static inline void bcm2835_gpio_setfunc(bcm2835_gpio_regmap_t *regmap, uint8_t pin, uint8_t func) {
    if(pin > 53) { return; }

    int idx = pin / 10;
    int off = (pin % 10) * 3;

    regmap->function_select[idx] =
        (regmap->function_select[idx] & ~(BCM2835_GPIO_FUNCTIONSELECT__MASK << off)) |
        ((func & BCM2835_GPIO_FUNCTIONSELECT__MASK) << off);
}

/* TODO: Define this elsewhere */
static inline void _gpio_delay_cycles(int32_t cycles) {
    asm volatile("__delay_%=: subs %[cycles], %[cycles], #1; bne __delay_%=\n"
                 : "=r"(cycles)
                 : [cycles]"0"(cycles)
                 : "cc");
}

static inline void bcm2835_gpio_setpull(bcm2835_gpio_regmap_t *regmap, uint8_t pin, uint8_t pull) {
    if(pin > 53) { return; }

    int idx = pin / 32;
    int off = pin % 32;

    regmap->pull = pull;
    _gpio_delay_cycles(150);
    regmap->pull_clock[idx] = (1UL << off);
    _gpio_delay_cycles(150);
    regmap->pull_clock[idx] = 0;
}

static inline void bcm2835_gpio_set(bcm2835_gpio_regmap_t *regmap, uint8_t pin, uint8_t value) {
    if(pin > 53) { return; }
    int idx = pin / 32;
    int off = pin % 32;
    if(value) {
        regmap->output_set[idx] = (1UL << off);
    } else {
        regmap->output_clear[idx] = (1UL << off);
    }
}

static inline int bcm2835_gpio_get(bcm2835_gpio_regmap_t *regmap, uint8_t pin) {
    if(pin > 53) { return 0; }
    int idx = pin / 32;
    int off = pin % 32;
    return (regmap->pin_level[idx] & (1UL << off)) ? 1 : 0;
}


/* TODO: Remove */
static inline void bcm2835_gpio_debug(uint8_t val) {
    bcm2835_gpio_regmap_t *gpio =
        (bcm2835_gpio_regmap_t *)(BROADCOM_BCM2837_PERIPHBASE_PI2 + BROADCOM_BCM2837_PERIPHBASE_OFF_GPIO);

    bcm2835_gpio_set(gpio, GPIOPIN_DEBUG_0, val & 0x01);
    bcm2835_gpio_set(gpio, GPIOPIN_DEBUG_1, val & 0x02);
    bcm2835_gpio_set(gpio, GPIOPIN_DEBUG_2, val & 0x04);
    bcm2835_gpio_set(gpio, GPIOPIN_DEBUG_3, val & 0x08);
    bcm2835_gpio_set(gpio, GPIOPIN_DEBUG_4, val & 0x10);
}

static inline void bcm2835_gpio_debug_init() {
    bcm2835_gpio_regmap_t *gpio =
        (bcm2835_gpio_regmap_t *)(BROADCOM_BCM2837_PERIPHBASE_PI2 + BROADCOM_BCM2837_PERIPHBASE_OFF_GPIO);
    
    bcm2835_gpio_setfunc(gpio, GPIOPIN_DEBUG_0, BCM2835_GPIO_FUNCTIONSELECT_OUTPUT);
    bcm2835_gpio_setfunc(gpio, GPIOPIN_DEBUG_1, BCM2835_GPIO_FUNCTIONSELECT_OUTPUT);
    bcm2835_gpio_setfunc(gpio, GPIOPIN_DEBUG_2, BCM2835_GPIO_FUNCTIONSELECT_OUTPUT);
    bcm2835_gpio_setfunc(gpio, GPIOPIN_DEBUG_3, BCM2835_GPIO_FUNCTIONSELECT_OUTPUT);
    bcm2835_gpio_setfunc(gpio, GPIOPIN_DEBUG_4, BCM2835_GPIO_FUNCTIONSELECT_OUTPUT);

    bcm2835_gpio_debug(0xFF);
}

#endif
