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

#define DECIMAL_SPEC(c)  (c == 'd' || c == 'i') //!< Signed integer
#define UNSIGNED_SPEC(c) (c == 'u')             //!< Unsigned integer
#define FDOUBLE_SPEC(c)  (c == 'f' || c == 'F') //!< Double in normal (fixed) form
#define EDOUBLE_SPEC(c)  (c == 'e' || c == 'E') //!< Double in standard (exponent) form
#define GDOUBLE_SPEC(c)  (c == 'g' || c == 'G') //!< Double in either normal or standard form, depending on magnitude
#define HEX_SPEC(c)      (c == 'x' || c == 'X') //!< Unsigned integer as hexadecimal
#define OCTAL_SPEC(c)    (c == 'o')             //!< Unsigned integer as octal
#define STRING_SPEC(c)   (c == 's')             //!< C-style string
#define CHAR_SPEC(c)     (c == 'c')             //!< Character
#define POINTER_SPEC(c)  (c == 'p')             //!< Pointer in hexadecimal
#define HEXFLOAT_SPEC(c) (c == 'a' || c == 'A') //!< Double as hexadecimal
#define INUM_SPEC(c)     (c == 'n')             //!< Place number of characters outputted to a pointer

#define IS_SPEC(c) \
	(DECIMAL_SPEC(c) || UNSIGNED_SPEC(c) || FDOUBLE_SPEC(c) || EDOUBLE_SPEC(c) || \
	GDOUBLE_SPEC(c) || HEX_SPEC(c) || OCTAL_SPEC(c) || STRING_SPEC(c) ||          \
	CHAR_SPEC(c) || POINTER_SPEC(c) || HEXFLOAT_SPEC(c) || INUM_SPEC(c))


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


#endif