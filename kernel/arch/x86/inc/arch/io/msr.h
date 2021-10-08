#ifndef ARCH_IO_MSR
#define ARCH_IO_MSR

#include <arch/io/cpuid.h>

/**
 * @brief Check if the CPU suports the MSR instructions
 * 
 * @return 1 if supported, else 0
 */
static inline int msr_avail(void) {
    return cpuid_featurecheck_edx(CPUIDFEATURE_EDX_MSR);
}

typedef enum {
    MSRREG_APICBASE = 0x0000001B
} msr_register_e;

/**
 * @brief Read current value of MSR
 * 
 * @param reg MSR to read
 * @return value of MSR
 */
static inline uint64_t msr_read(msr_register_e reg) {
    uint32_t eax, edx;
    asm volatile("rdmsr" :
                 "=a"(eax), "=d"(edx) :
                 "c"(reg));
    return ((uint64_t)edx << 32) | eax;
}

/**
 * @brief Write value to MSR
 * 
 * @param reg MSR to write to
 * @param value Value to write to MSR
 */
static inline void msr_write(msr_register_e reg, uint64_t value) {
    uint32_t eax = (uint32_t)value;
    uint32_t edx = (uint32_t)(value >> 32);
    asm volatile("wrmsr" : :
                 "a"(eax), "d"(edx), "c"(reg));
}


#endif
