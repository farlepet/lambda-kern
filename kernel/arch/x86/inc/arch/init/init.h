/** \file init.h
 *  \brief Defines initialization function(s) for the target architecture.
 *
 */

#ifndef ARCH_X86_INIT_H
#define ARCH_X86_INIT_H

#include <arch/boot/multiboot.h>

void arch_init(mboot_t *mboot_head);

#endif