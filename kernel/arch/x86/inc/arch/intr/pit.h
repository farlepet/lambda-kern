#ifndef PIT_H
#define PIT_H

#include <types.h>

#include <hal/timer/timer.h>


/**
 * \brief Initialize the PIT.
 * Initialized the PIT using the supplied frequency if possible.
 * @param freq frequency in Hz
 */
void pit_init(uint32_t freq);

void pit_create_timerdev(hal_timer_dev_t *dev);

#endif
