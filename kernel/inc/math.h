#ifndef MATH_H
#define MATH_H

#include <types.h>

#if __has_builtin(__builtin_addc)
/**
 * Add two arbitrarily large numbers
 *
 * @param aug augend
 * @param add addend
 * @param sum sum
 * @param size size of number in 4 byte blocks
 */
int u_arbi_add(uint32_t *aug, uint32_t *add, uint32_t *sum, int size);
#endif


#if __has_builtin(__builtin_subc)
/**
 * Subtract two arbitrarily large numbers
 *
 * @param min minuend
 * @param sub subtrahend
 * @param diff difference
 * @param size size of number in 4 byte blocks
 */
int u_arbi_sub(uint32_t *min, uint32_t *sub, uint32_t *diff, int size);
#endif

#if (PLATFORM_BITS < PLATFORM_BITS_64)
/**
 * @brief Divides two 6-bit values, and optionally stores the remainder. Used
 * to allow use of 64-bit numbers on 32-bit platforms.
 * 
 * @note This is not an effecient method, so 64-bit division should be used
 * sparingly in 32-bit platforms.
 * 
 * @param a Dividend
 * @param b Divsior
 * @param rem Remainder
 * @return uint64_t Quotient
 */
uint64_t udiv64(uint64_t a, uint64_t b, uint64_t *rem);
#else
static inline uint64_t udiv64(uint64_t a, uint64_t b, uint64_t *rem) {
    if(rem) {
        *rem = a % b;
    }
    return a / b;
}
#endif


#endif