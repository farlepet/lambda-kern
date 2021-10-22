#ifndef LAMBDA_PLATFORMS_H
#define LAMBDA_PLATFORMS_H

#define PLATFORM_ARCH_X86    (1)
#define PLATFORM_ARCH_RISCV  (3)
#define PLATFORM_ARCH_ARM32  (4)
#define PLATFORM_ARCH_ARM64  (5)

/* TODO: Potentially move the following to their respective architectures, as
 * they are platform-specific */
#define PLATFORM_CPU_X86           (1)
#define PLATFORM_CPU_X86_64        (2)
#define PLATFORM_CPU_RISCV_RV64I   (3)

#define PLATFORM_HW_PC              (1)
#define PLATFORM_HW_KENDRYTE_K210   (2)

#endif
