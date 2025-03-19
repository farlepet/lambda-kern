#include <errno.h>
#include <string.h>

#include <arch/io/ioport.h>
#include <arch/intr/idt.h>
#include <arch/intr/pic.h>
#include <arch/intr/pit.h>

#include <proc/mtask.h>
#include <intr/intr.h>
#include <time/time.h>

static void (*pit_callback)(void) = NULL;

static int timerdev_setfreq(void *data, uint8_t idx, uint32_t freq);
static int timerdev_attach(void *data, uint8_t idx, void (*callback)(void));

void pit_create_timerdev(hal_timer_dev_t *dev) {
    memset(dev, 0, sizeof(hal_timer_dev_t));

    dev->setfreq   = timerdev_setfreq;
    dev->setperiod = NULL;
    dev->attach    = timerdev_attach;

    dev->cap = HAL_TIMERDEV_CAP_VARFREQ | HAL_TIMERDEV_CAP_CALLBACK;
}

/**
 * \brief PIT interrupt handler.
 * The main part of the PIT interrupt handler, called from pit_int().
 * @see pit_int
 */
static void _pit_handler(intr_handler_hand_t *hdlr) {
    (void)hdlr;

    /* TODO: Determine from timer settings. */
    time_update(10);
    
    outb(0x20, 0x20);

    if(pit_callback) {
        pit_callback();
    }
}

/**
 * \brief Creates a PIT reload value.
 * Creates a PIT reload value from the specified frequency.
 * @param freq frequency in Hz
 */
static __inline uint32_t get_reload(uint32_t freq)
{
    if(freq < 18)      return 0x10000;   // Is the frequency too small?
    if(freq > 1193181) return 0x01;     // Is the frequency too large?
    return (1193180 / freq);           // If not, compute the reload value
}

/* @todo Possibly make this dynamically allocated */
static intr_handler_hand_t _pit_int_hdlr = {
    .callback = _pit_handler,
    .data     = NULL
};

void pit_init(uint32_t freq) {
    uint32_t reload = get_reload(freq);
    outb(0x43, 0x34);
    outb(0x40, (uint8_t)reload);
    outb(0x40, (uint8_t)(reload >> 8));

    interrupt_attach(INTR_TIMER, &_pit_int_hdlr);
    pic_irq_enable(PIC_IRQ_TIMER);
}

static int timerdev_setfreq(void __unused *data, uint8_t idx, uint32_t freq) {
    if(idx != 0) {
        return -EINVAL;
    }

    if(freq < 18 || freq > 1193181) return -1;

    uint32_t reload = get_reload(freq);
    outb(0x43, 0x34);
    outb(0x40, (uint8_t)reload);
    outb(0x40, (uint8_t)(reload >> 8));

    return 0;
}

static int timerdev_attach(void __unused *data, uint8_t idx, void (*callback)(void)) {
    if(idx != 0) {
        return -EINVAL;
    }
    
    if(pit_callback) {
        /* Presently only support a single callback */
        return -EUNSPEC;
    }

    pit_callback = callback;

    return 0;
}
