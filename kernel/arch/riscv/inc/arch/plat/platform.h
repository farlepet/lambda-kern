#ifndef ARCH_PLAT_PLATFORM_H
#define ARCH_PLAT_PLATFORM_H

#if defined(__LAMBDA_PLATFORM_CPU__)
#  if   (__LAMBDA_PLATFORM_CPU__ == PLATFORM_CPU_RISCV_RV64I)
#    include <arch/plat/cpu/rv64i.h>
#  endif
#endif /* __LAMBDA_PLATFORM_CPU__ */

#if defined(__LAMBDA_PLATFORM_HW__)
#  if   (__LAMBDA_PLATFORM_HW__ == PLATFORM_HW_KENDRYTE_K210)
#    include <arch/plat/hw/kendryte_k210.h>
#  endif
#endif /* __LAMBDA_PLATFORM_HW__ */

#endif