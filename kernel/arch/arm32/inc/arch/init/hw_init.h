#ifndef ARCH_ARM32_HW_INIT_H
#define ARCH_ARM32_HW_INIT_H

/* These are common fuctions that are provided by the HW platforms. See
 * hw/<hw>/init/hwinit.c */

#include <stdint.h>

/**
 * @brief Initialize console (typically serial) used by kernel for early boot messages
 */
int hw_init_console(void);

/**
 * @brief Initialize interrupt controller
 */
int hw_init_interrupts(void);

/**
 * @brief Initialize memory management
 */
int hw_init_mm(void);

/**
 * @brief Initialize timers for timekeeping and task switching
 * 
 * @param rate Rate of clock keeping kernel time
 */
int hw_init_timer(uint32_t rate);

#endif
