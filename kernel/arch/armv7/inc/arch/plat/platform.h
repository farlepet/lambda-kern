#ifndef ARCH_PLAT_PLATFORM_H
#define ARCH_PLAT_PLATFORM_H

#if defined(__LAMBDA_PLATFORM_CPU__)
#  if   (__LAMBDA_PLATFORM_CPU__ == PLATFORM_CPU_ARM_CORTEX_A9)
#    include <arch/plat/cpu/cortex_a9.h>
#  elif (__LAMBDA_PLATFORM_CPU__ == PLATFORM_CPU_ARM_CORTEX_A7)
#    include <arch/plat/cpu/cortex_a7.h>
#  endif
#endif /* __LAMBDA_PLATFORM_CPU__ */

#if defined(__LAMBDA_PLATFORM_HW__)
#  if   (__LAMBDA_PLATFORM_HW__ == PLATFORM_HW_ARM_VEXPRESS_A9)
#    include <arch/plat/hw/vexpress_a9.h>
#  elif (__LAMBDA_PLATFORM_HW__ == PLATFORM_HW_ARM_ALLWINNER_V3s)
#    include <arch/plat/hw/allwinner_v3s.h>
#  endif
#endif /* __LAMBDA_PLATFORM_HW__ */

#endif