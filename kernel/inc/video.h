#ifndef VIDEO_H
#define VIDEO_H

#include <types.h>

/**
 * \brief Prints a single character.
 * Uses the current architecture's put function
 * @param c the input character
 */
void kput(char c);

/**
 * \brief Prints a string of characters.
 * Uses the current architecture's print function
 * @param str the input string
 */
void kprint(char *str);

/**
 * \brief Prints a number using the specified base.
 * Prints a number using any base between 2 and 16, inclusive.
 * @param n number to be printed
 * @param base base to use when printing the number
 * @see kput
 */
void kprintnum(u32 num, int base);

#endif