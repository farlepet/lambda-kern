#ifndef KEYB_H
#define KEYB_H

#include <types.h>

/**
 * Initializes the keyboard.
 *  * Checks that the keyboard is in working condition
 *  * Sets the keyboard interrupt handler
 *  * Enables the keyboard IRQ
 *  * Creates and adds an input device driver entry corresponding to this keyboard
 */
void keyb_init(void);

#endif