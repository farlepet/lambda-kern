#include <math.h>

/**
 * Add two arbitrarily large numbers
 *
 * @param aug augend
 * @param add addend
 * @param sum sum
 * @param size size of number in 4 byte blocks
 */
int u_arbi_add(u32 *aug, u32 *add, u32 *sum, int size)
#if __has_builtin(__builtin_addc) // This makes our life a LOT easier
{
	u32 carry_in  = 0;
	u32 carry_out = 0;
	int i = 0;
	for(; i < size; i++)
	{
		carry_in = carry_out;
		sum[i] = __builtin_addc(aug[i], add[i], carry_in, &carry_out);
	}
	return (int)carry_out;
}
#else
{
	return 0;
}
#endif


/**
 * Subtract two arbitrarily large numbers
 *
 * @param min minuend
 * @param sub subtrahend
 * @param diff difference
 * @param size size of number in 4 byte blocks
 */
int u_arbi_sub(u32 *min, u32 *sub, u32 *diff, int size)
#if __has_builtin(__builtin_subc) // This makes our life a LOT easier
{
	u32 carry_in  = 0;
	u32 carry_out = 0;
	int i = 0;
	for(; i < size; i++)
	{
		carry_in = carry_out;
		diff[i] = __builtin_subc(min[i], sub[i], carry_in, &carry_out);
	}
	return (int)carry_out;
}
#else
{
	return 0;
}
#endif