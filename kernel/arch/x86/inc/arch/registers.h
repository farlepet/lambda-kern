#ifndef ARCH_REGISTERS_H
#define ARCH_REGISTERS_H

#include <stdint.h>

/* CR0 */
#define CR0_FLAG_PE         (1UL <<  0) /** Protected Mode Enable */
#define CR0_FLAG_MP         (1UL <<  1) /** Monitor Coprocessor (WAIT/FWAIT) */
#define CR0_FLAG_EM         (1UL <<  2) /** Emulation - Allows generating an exception on x87/vector instructions when not present to enable software emulation */
#define CR0_FLAG_TS         (1UL <<  3) /** Task Switched - Allows on-demand saving of x87/vector registers */
#define CR0_FLAG_ET         (1UL <<  4) /** Extension Type */
#define CR0_FLAG_NE         (1UL <<  5) /** x87 Numeric Error Enable */
#define CR0_FLAG_WP         (1UL << 16) /** Write Protect in Supervisor Mode */
#define CR0_FLAG_AM         (1UL << 18) /** Alignment Check Enable */
#define CR0_FLAG_NW         (1UL << 29) /** Not Write-Through */
#define CR0_FLAG_CD         (1UL << 30) /** Cache Disable */
#define CR0_FLAG_PG         (1UL << 31) /** Paging Enable */

static inline uint32_t register_cr0_read(void) {
    uint32_t val;
    asm volatile("mov %%cr0, %0": "=b"(val));
    return val;
}

static inline void register_cr0_write(uint32_t val) {
    asm volatile("mov %0, %%cr0":: "b"(val));
}

/* CR3 */
#define CR3_FLAG_PWT        (1UL <<  3) /** Page-level Write-Through */
#define CR3_FLAG_PCD        (1UL <<  4) /** Page-level Cache Disable */

static inline uint32_t register_cr3_read(void) {
    uint32_t val;
    asm volatile("mov %%cr3, %0": "=b"(val));
    return val;
}

static inline void register_cr3_write(uint32_t val) {
    asm volatile("mov %0, %%cr3":: "b"(val));
}

/* CR4 */
#define CR4_FLAG_VME        (1UL <<  0) /** v8086 Mode Extensions */
#define CR4_FLAG_PVI        (1UL <<  1) /** Protected-Mode Virtual Interrupts */
#define CR4_FLAG_TSD        (1UL <<  2) /** Time Stamp Disable - Restricts RDTSC/RDTSCP instructions to supervisor mode */
#define CR4_FLAG_DE         (1UL <<  3) /** Debugging Extensions */
#define CR4_FLAG_PSE        (1UL <<  4) /** Page Size Extensions */
#define CR4_FLAG_PAE        (1UL <<  5) /** Physical Address Extension */
#define CR4_FLAG_MCE        (1UL <<  6) /** Machine-Check Enable */
#define CR4_FLAG_PGE        (1UL <<  7) /** Enable Global Pages */
#define CR4_FLAG_PCE        (1UL <<  8) /** Performance-Monitoring Counter Enable */
#define CR4_FLAG_OSFXSR     (1UL <<  9) /** Operating System Support for FXSAVE and FXSTOR Instructions */
#define CR4_FLAG_OSXMMEXCPT (1UL << 10) /** Operating System Support for Unmasked SIMD Floating Point Exceptions */
#define CR4_FLAG_UMIP       (1UL << 11) /** User-Mode Instruction Prevention - Prohibits SGDT, SIDT, SLDT, and STR outside of ring 0 */
#define CR4_FLAG_VMXE       (1UL << 13) /** VMX Enable */
#define CR4_FLAG_SMXE       (1UL << 14) /** SMX Enable */
#define CR4_FLAG_FSGBASE    (1UL << 16) /** FSGSBASE Enable */
#define CR4_FLAG_PCIDE      (1UL << 17) /** PCID Enable */
#define CR4_FLAG_OSXSAVE    (1UL << 18) /** XSAVE and Processor Extended States Enable */
#define CR4_FLAG_KL         (1UL << 19) /** Key-Locker Enable */
#define CR4_FLAG_SMEP       (1UL << 20) /** Supervisor Mode Execution Prevention Enable */
#define CR4_FLAG_SMAP       (1UL << 21) /** Supervisor Mode Access Prevention Enable */
#define CR4_FLAG_PKE        (1UL << 22) /** Enable protection keys for user-mode pages */
#define CR4_FLAG_CET        (1UL << 23) /** Control-flow Enforcement Technology */
#define CR4_FLAG_PKS        (1UL << 24) /** Enable Protection Keys for Supervisor-mode pages */

static inline uint32_t register_cr4_read(void) {
    uint32_t val;
    asm volatile("mov %%cr4, %0": "=b"(val));
    return val;
}

static inline void register_cr4_write(uint32_t val) {
    asm volatile("mov %0, %%cr4":: "b"(val));
}

#endif
