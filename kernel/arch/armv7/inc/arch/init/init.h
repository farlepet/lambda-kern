/** \file init.h
 *  \brief Defines initialization function(s) for the target architecture.
 *
 */

#ifndef ARCH_ARMV7_INIT_H
#define ARCH_ARMV7_INIT_H

#include <hal/intr/int_ctlr.h>

extern hal_intctlr_dev_t intctlr;

void arch_init(void);

#endif