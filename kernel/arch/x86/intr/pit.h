#ifndef PIT_H
#define PIT_H

#include <types.h>

/**
 * \brief Initialize the PIT.
 * Initialized the PIT using the supplied frequency if possible.
 * @param freq frequency in Hz
 */
void pit_init(u32 freq);

#endif