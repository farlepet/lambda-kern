#ifndef HAL_CLOCK_CLOCK_H
#define HAL_CLOCK_CLOCK_H

#include <types.h>

/* Max representable frequency is 4.29 GHz, might need to increase to 64-bit */
typedef uint32_t hal_clkfreq_t;

typedef struct {
    /* TODO: Add functionality to modify clock and get clock information. */

    hal_clkfreq_t freq; /* Clock frequency */
} hal_clock_dev_t;

#endif
