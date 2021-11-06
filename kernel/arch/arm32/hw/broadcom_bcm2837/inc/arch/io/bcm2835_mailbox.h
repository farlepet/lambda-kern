#ifndef ARCH_ARM32_INTR_BCM2835_MAILBOX_H
#define ARCH_ARM32_INTR_BCM2835_MAILBOX_H

#include <stdint.h>

#include <hal/intr/int_ctlr.h>

typedef volatile struct {
    struct {
        uint32_t rw;
        uint32_t _reserved[3];
        uint32_t peek;
        uint32_t sender;
        uint32_t status;
#define BCM2835_MAILMOX_STATUS_EMPTY (1UL << 30)
#define BCM2835_MAILMOX_STATUS_FULL  (1UL << 31)
        uint32_t config;
    } box[2];
} bcm2835_mailbox_regmap_t;

typedef enum {
    BCM2835_MAILBOX_CHAN_POWER           = 0,
    BCM2835_MAILBOX_CHAN_FRAMEBUFFER     = 1,
    BCM2835_MAILBOX_CHAN_VIRTUALUART     = 2,
    BCM2835_MAILBOX_CHAN_VCHIQ           = 3,
    BCM2835_MAILBOX_CHAN_LEDS            = 4,
    BCM2835_MAILBOX_CHAN_BUTTONS         = 5,
    BCM2835_MAILBOX_CHAN_TOUCHSCREEN     = 6,
    
    BCM2835_MAILBOX_CHAN_PROPTAGS_TOVC   = 8,
    BCM2835_MAILBOX_CHAN_PROPTAGS_FROMVC = 9,
} bcm2835_mailbox_channel_e;

static inline void bcm2835_mailbox_write(bcm2835_mailbox_regmap_t *regmap, uint8_t channel, uint32_t data) {
    while(regmap->box[1].status & BCM2835_MAILMOX_STATUS_FULL) {}
    regmap->box[1].rw = (data & 0xFFFFFFF0) | (channel & 0x0F);
}

static inline uint32_t bcm2835_mailbox_read(bcm2835_mailbox_regmap_t *regmap, uint8_t channel) {
    uint32_t read;
    do {
        while(regmap->box[0].status & BCM2835_MAILMOX_STATUS_EMPTY) {}
        read = regmap->box[0].rw;
    } while((read & 0x0F) != channel);
    
    return (read & 0xFFFFFFF0);
}

#endif
