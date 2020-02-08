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
int u_arbi_add(uint32_t *aug, uint32_t *add, uint32_t *sum, int size);


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