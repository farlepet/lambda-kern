#ifndef ARCH_IO_CPUID_H
#define ARCH_IO_CPUID_H

#include <stdint.h>

typedef enum {
    CPUIDOP_MAXLEVELANDVENDOR = 0x00000000,
    CPUIDOP_FEATURES          = 0x00000001,

    CPUIDOP_SERIAL            = 0x00000003
} cpuid_op_e;


/**
 * @brief Check if CPU supports CPUID instruction
 *
 * @note This may inappropriately return 0 on certain old x86 clones
 *
 * @return 1 if supported, else 0
 */
static inline int cpuid_avail(void) {
    uint32_t diff;

    asm volatile("pushfl                   \n"
                 /* Invert ID bit */
                 "pushfl                   \n"
                 "xorl $0x00200000, (%%esp)\n"
                 "popfl                    \n"
                 /* Check if ID was properly flipped */
                 "pushfl                   \n"
                 "popl %%eax               \n"
                 "xorl %%eax, (%%esp)      \n"
                 /* Enforce CPUID enable, for the few CPUs that require it */
                 "orl  $0x00200000, (%%esp)\n"
                 "popfl                    \n"
                 : "=r"(diff));

    /* If ID bit could be flipped, CPUID is supported */
    return (diff & 0x00200000) ? 1 : 0;
}

/**
 * @brief Read CPUID
 * 
 * @param op CPUID operation
 * @param ret CPUID result, array of EAX, EBX, ECX, EDX
 */
static inline void cpuid(cpuid_op_e op, uint32_t ret[4]) {
    asm volatile("cpuid" :
                 "=a"(ret[0]),"=b"(ret[1]),"=c"(ret[2]),"=d"(ret[3]) :
                 "a"(op));
}

#define CPUIDFEATURE_ECX_HV      (1UL << 31)
#define CPUIDFEATURE_ECX_RDRAND  (1UL << 30)
#define CPUIDFEATURE_ECX_F16C    (1UL << 29)
#define CPUIDFEATURE_ECX_AVX     (1UL << 28)
#define CPUIDFEATURE_ECX_OSXSAVE (1UL << 27)
#define CPUIDFEATURE_ECX_XSAVE   (1UL << 26)
#define CPUIDFEATURE_ECX_AES     (1UL << 25)
#define CPUIDFEATURE_ECX_TSCD    (1UL << 24)
#define CPUIDFEATURE_ECX_POPCNT  (1UL << 23)
#define CPUIDFEATURE_ECX_MOVBE   (1UL << 22)
#define CPUIDFEATURE_ECX_X2APIC  (1UL << 21)
#define CPUIDFEATURE_ECX_SSE42   (1UL << 20)
#define CPUIDFEATURE_ECX_SSE41   (1UL << 19)
#define CPUIDFEATURE_ECX_DCA     (1UL << 18)
#define CPUIDFEATURE_ECX_PCID    (1UL << 17)
#define CPUIDFEATURE_ECX_PDCM    (1UL << 15)
#define CPUIDFEATURE_ECX_ETPRD   (1UL << 14)
#define CPUIDFEATURE_ECX_CX16    (1UL << 13)
#define CPUIDFEATURE_ECX_FMA     (1UL << 12)
#define CPUIDFEATURE_ECX_SDBG    (1UL << 11)
#define CPUIDFEATURE_ECX_CID     (1UL << 10)
#define CPUIDFEATURE_ECX_SSSE3   (1UL <<  9)
#define CPUIDFEATURE_ECX_TM2     (1UL <<  8)
#define CPUIDFEATURE_ECX_EST     (1UL <<  7)
#define CPUIDFEATURE_ECX_SMX     (1UL <<  6)
#define CPUIDFEATURE_ECX_VMX     (1UL <<  5)
#define CPUIDFEATURE_ECX_DSCPL   (1UL <<  4)
#define CPUIDFEATURE_ECX_MON     (1UL <<  3)
#define CPUIDFEATURE_ECX_DTES64  (1UL <<  2)
#define CPUIDFEATURE_ECX_PCLMUL  (1UL <<  1)
#define CPUIDFEATURE_ECX_SSE3    (1UL <<  0)

#define CPUIDFEATURE_EDX_PBE     (1UL << 31)
#define CPUIDFEATURE_EDX_IA64    (1UL << 30)
#define CPUIDFEATURE_EDX_TM1     (1UL << 29)
#define CPUIDFEATURE_EDX_HTT     (1UL << 28)
#define CPUIDFEATURE_EDX_SS      (1UL << 27)
#define CPUIDFEATURE_EDX_SSE2    (1UL << 26)
#define CPUIDFEATURE_EDX_SSE     (1UL << 25)
#define CPUIDFEATURE_EDX_FXSR    (1UL << 24)
#define CPUIDFEATURE_EDX_MMX     (1UL << 23)
#define CPUIDFEATURE_EDX_ACPI    (1UL << 22)
#define CPUIDFEATURE_EDX_DTES    (1UL << 21)
#define CPUIDFEATURE_EDX_CLFL    (1UL << 19)
#define CPUIDFEATURE_EDX_PSN     (1UL << 18)
#define CPUIDFEATURE_EDX_PSE36   (1UL << 17)
#define CPUIDFEATURE_EDX_PAT     (1UL << 16)
#define CPUIDFEATURE_EDX_CMOV    (1UL << 15)
#define CPUIDFEATURE_EDX_MCA     (1UL << 14)
#define CPUIDFEATURE_EDX_PGE     (1UL << 13)
#define CPUIDFEATURE_EDX_MTRR    (1UL << 12)
#define CPUIDFEATURE_EDX_SEP     (1UL << 11)
#define CPUIDFEATURE_EDX_APIC    (1UL <<  9)
#define CPUIDFEATURE_EDX_CX8     (1UL <<  8)
#define CPUIDFEATURE_EDX_MCE     (1UL <<  7)
#define CPUIDFEATURE_EDX_PAE     (1UL <<  6)
#define CPUIDFEATURE_EDX_MSR     (1UL <<  5)
#define CPUIDFEATURE_EDX_TSC     (1UL <<  4)
#define CPUIDFEATURE_EDX_PSE     (1UL <<  3)
#define CPUIDFEATURE_EDX_DE      (1UL <<  2)
#define CPUIDFEATURE_EDX_VME     (1UL <<  1)
#define CPUIDFEATURE_EDX_FPU     (1UL <<  0)

/**
 * @brief Check if feature(s) reported in ECX register of CPUID 0x01 are present
 * 
 * @param features Feature(s) to check for
 * @return 1 if present, else 0
 */
static inline int cpuid_featurecheck_ecx(uint32_t features) {
    uint32_t ret[4];
    cpuid(CPUIDOP_FEATURES, ret);
    return ((ret[2] & features) == features);
}

/**
 * @brief Check if feature(s) reported in EDX register of CPUID 0x01 are present
 * 
 * @param features Feature(s) to check for
 * @return 1 if present, else 0
 */
static inline int cpuid_featurecheck_edx(uint32_t features) {
    uint32_t ret[4];
    cpuid(CPUIDOP_FEATURES, ret);
    return ((ret[3] & features) == features);
}

#endif
