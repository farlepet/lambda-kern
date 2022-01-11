#ifndef ARCH_ARM32_HW_INIT_H
#define ARCH_ARM32_HW_INIT_H

#include <stdint.h>

/* These are common fuctions that are provided by the HW platforms. See
 * hw/<hw>/init/hwinit.c */

/**
 * @brief Initialize timers for timekeeping and task switching
 * 
 * @param rate Rate of clock keeping kernel time
 */
int hw_init_timer(uint32_t rate);

#endif
