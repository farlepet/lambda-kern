#include <math.h>

/**
 * Add two arbitrarily large numbers
 *
 * @param aug augend
 * @param add addend
 * @param sum sum
 * @param size size of number in 4 byte blocks
 * @return 1 if implemented
 */
int u_arbi_add(uint32_t *aug, uint32_t *add, uint32_t *sum, int size)
#if __has_builtin(__builtin_addc) // This makes our life a LOT easier
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
#else
{
	// TODO: Implement for GCC / other compilers
	(void)aug;
	(void)add;
	(void)sum;
	(void)size;
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
 * @return 1 if implemented
 */
int u_arbi_sub(uint32_t *min, uint32_t *sub, uint32_t *diff, int size)
#if __has_builtin(__builtin_subc) // This makes our life a LOT easier
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
#else
{
	(void)min;
	(void)sub;
	(void)diff;
	(void)size;
	return 0;
}
#endif