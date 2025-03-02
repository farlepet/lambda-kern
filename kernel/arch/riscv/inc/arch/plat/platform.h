#ifndef ARCH_PLAT_PLATFORM_H
#define ARCH_PLAT_PLATFORM_H

#if defined(CONFIG_ARCH_CPU_RISCV_RV64I)
#  include <arch/plat/cpu/rv64i.h>
#endif

#if defined(CONFIG_ARCH_HW_KENDRYTE_K210)
#  include <arch/plat/hw/kendryte_k210.h>
#endif

#endif
