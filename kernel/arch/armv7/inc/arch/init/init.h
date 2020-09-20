/** \file init.h
 *  \brief Defines initialization function(s) for the target architecture.
 *
 */

#ifndef ARCH_ARMV7_INIT_H
#define ARCH_ARMV7_INIT_H

#include <multiboot.h>

void arch_init(struct multiboot_header *mboot_head);

#endif