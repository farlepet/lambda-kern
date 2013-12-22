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


#define FMT_SPEC '%' //!< Format specifier character


#define ZERO_ALL_VID()  \
	do {                \
		is_in_spec = 0; \
		size       = 0; \
		width      = 0; \
		precision  = 0; \
		showsign   = 0; \
		signspace  = 0; \
		leftalign  = 0; \
		padzeros   = 0; \
	} while(0);


/**
	* \brief Creates a string based on input format string and arguments.
	* Uses `print` to convert the format string and any number of arguments to
	* an output string.
	* @param out output string
	* @param format format string
	* @param ... argument list
	* @return the number of characters placed in `out`
	* @see print
	*/
int sprintf(char *out, char *format, ...);

/**
 * \brief Creates and prints a string based on input format string and arguments.
 * Uses `print` to convert the format string and any number of arguments to
 * a string then prints that string to the screen.
 * @param format format string
 * @param ... argument list
 * @return the number of characters printed
 * @see print
 */
int kprintf(char *format, ...);

/**
 * \brief Creates and prints a string based on input format string and arguments.
 * Uses `print` to convert the format string and any number of arguments in varg to
 * a string then prints that string to the screen.
 * @param format format string
 * @param varg argument list
 * @return the number of characters printed
 * @see print
 */
int kprintv(char *format, ptr_t *varg);

#endif