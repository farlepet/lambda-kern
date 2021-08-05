#ifndef LAMBDA_PLATFORMS_H
#define LAMBDA_PLATFORMS_H

/* TODO: Use __LAMBDA_PLATFORM_ARCH__ rather than ARCH_* everywhere */
#define PLATFORM_ARCH_X86    (1)
#define PLATFORM_ARCH_X86_64 (2)
#define PLATFORM_ARCH_RISCV  (3)
#define PLATFORM_ARCH_ARMV7  (4)

/* TODO: Potentially move the following to their respective architectures, as
 * they are platform-specific */
#define PLATFORM_CPU_X86           (1)
#define PLATFORM_CPU_X86_64        (2)
#define PLATFORM_CPU_RISCV_RV64I   (3)
#define PLATFORM_CPU_ARM_CORTEX_A9 (4)

#define PLATFORM_HW_PC              (1)
#define PLATFORM_HW_KENDRYTE_K210   (2)
#define PLATFORM_HW_ARM_VEXPRESS_A9 (3)

#endif
