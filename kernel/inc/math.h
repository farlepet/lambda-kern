#ifndef MATH_H
#define MATH_H

#include <types.h>

/**
 * Add two arbitrarily large numbers
 *
 * @param aug augend
 * @param add addend
 * @param sum sum
 * @param size size of number in 4 byte blocks
 */
int u_arbi_add(u32 *aug, u32 *add, u32 *sum, int size);


/**
 * Subtract two arbitrarily large numbers
 *
 * @param min minuend
 * @param sub subtrahend
 * @param diff difference
 * @param size size of number in 4 byte blocks
 */
int u_arbi_sub(u32 *min, u32 *sub, u32 *diff, int size);

#endif