/** \file print.h
 *  \brief Contains functions dealing with printing to the VGA screen.
 *
 *  Contains functions that help print to and manage the VGA screen.
 */

#ifndef VGA_PRINT_H
#define VGA_PRINT_H

#include <types.h>

/**
 * \brief Clears VGA text.
 * Clears the first plane of VGA memory, effectively clearing all text from
 * the screen.
 */
void vga_clear();

/**
 * \brief Prints a single character to the VGA screen.
 * Checks if character is printable, if so it places it in VGA memory,
 * along with a color byte. If the character is not printable, it deals
 * with it accordingly
 * @param c the input character
 */
void vga_put(u8 c);


/**
 * \brief Prints a string of characters.
 * Prints every character in a character array , until it reaches
 * a NULL terminator.
 * @param str the string to print
 * @see vga_put
 */
void vga_print(char *str);

/**
 * \brief Prints a number using the specified base.
 * Prints a number using any base between 2 and 16, inclusive.
 * @param n number to be printed
 * @param base base to use when printing the number
 * @see vga_print
 */
void vga_printnum(u32 n, int base);

#endif