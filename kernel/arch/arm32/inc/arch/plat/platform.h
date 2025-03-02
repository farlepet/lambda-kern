#ifndef ARCH_PLAT_PLATFORM_H
#define ARCH_PLAT_PLATFORM_H

#include <stdint.h>

#define PLATFORM_CPU_ARM_CORTEX_A9  (1)
#define PLATFORM_CPU_ARM_CORTEX_A7  (2)
#define PLATFORM_CPU_ARM_CORTEX_A53 (3)

#define PLATFORM_HW_ARM_VEXPRESS_A9  (1)
#define PLATFORM_HW_ALLWINNER_V3S    (2)
#define PLATFORM_HW_BROADCOM_BCM2837 (3)

#if   defined(CONFIG_ARCH_CPU_ARM_CORTEX_A9)
#  include <arch/plat/cpu/cortex_a9.h>
#elif defined(CONFIG_ARCH_CPU_ARM_CORTEX_A7)
#  include <arch/plat/cpu/cortex_a7.h>
#elif defined(CONFIG_ARCH_CPU_ARM_CORTEX_A53)
#  include <arch/plat/cpu/cortex_a53.h>
#endif

#if   defined(CONFIG_ARCH_HW_ARM_VEXPRESS_A9)
#  include <arch/plat/hw/vexpress_a9.h>
#elif defined(CONFIG_ARCH_HW_ALLWINNER_V3S)
#  include <arch/plat/hw/allwinner_v3s.h>
#elif defined(CONFIG_ARCH_HW_BROADCOM_BCM2837)
#  include <arch/plat/hw/broadcom_bcm2837.h>
#endif

#endif
