#include <math.h>

#if __has_builtin(__builtin_addc)
int u_arbi_add(uint32_t *aug, uint32_t *add, uint32_t *sum, int size)
{
    uint32_t carry_in  = 0;
    uint32_t carry_out = 0;
    int i = 0;
    for(; i < size; i++)
    {
        carry_in = carry_out;
        sum[i] = __builtin_addc(aug[i], add[i], carry_in, &carry_out);
    }
    return 1;
}
#endif


#if __has_builtin(__builtin_subc)
int u_arbi_sub(uint32_t *min, uint32_t *sub, uint32_t *diff, int size)
{
    uint32_t carry_in  = 0;
    uint32_t carry_out = 0;
    int i = 0;
    for(; i < size; i++)
    {
        carry_in = carry_out;
        diff[i] = __builtin_subc(min[i], sub[i], carry_in, &carry_out);
    }
    return (int)carry_out;
}
#endif

#if (PLATFORM_BITS < PLATFORM_BITS_64)
uint64_t udiv64(uint64_t a, uint64_t b, uint64_t *rem) {
    /* This Basic implementation is based off the simple 128-bit division
     * algorithm found here: https://danlark.org/2020/06/14/128-bit-division/ */

    if(a < 0x100000000ULL) {
        /* Do fast 32-bit arithmetic when possible */
        if(rem) {
            *rem = (uint32_t)a % (uint32_t)b;
        }
        return (uint32_t)a / (uint32_t)b;
    }

    if(b > a) {
        *rem = a;
        return 0;
    }

    uint64_t q = 0; /* Quotient */

    /* Get difference in position of most-significant bits between dividend and
     * divisor
     * NOTE: (63 - clz(a)) - (63 - clz(b)) == clz(b) - clz(a) */
    int shift = __builtin_clzll(b | 1) - __builtin_clzll(a | 1);

    b <<= shift;

    while(shift >= 0) {
        q <<= 1;
        if(a >= b) {
            a -= b;
            q |= 1;
        }
        b >>= 1;
        shift--;
    }

    if(rem) {
        *rem = a;
    }
    return q;
}
#endif /* (PLATFORM_BITS < PLATFORM_BITS_64) */
