#ifndef IO_OUTPUT_H
#define IO_OUTPUT_H

#include <types.h>
#include <hal/io/char/char.h>

/**
 * @brief Set I/O device to write kernel output to
 *
 * @param dev Character device
 */
void output_set_dev(hal_io_char_dev_t *dev);


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
 * \brief Creates a string based on input format string and arguments.
 * Uses `print` to convert the format string and any number of arguments to
 * an output string.
 * @param out output string
 * @param format format string
 * @param ... argument list
 * @return the number of characters placed in `out`
 * @see print
 */
int sprintf(char *out, const char *format, ...);

/**
 * \brief Creates and prints a string based on input format string and arguments.
 * Uses `print` to convert the format string and any number of arguments to
 * a string then prints that string to the screen.
 * @param format format string
 * @param ... argument list
 * @return the number of characters printed
 * @see print
 */
int kprintf(const char *format, ...);

/**
 * \brief Creates and prints a string based on input format string and arguments.
 * Uses `print` to convert the format string and any number of arguments in varg to
 * a string then prints that string to the screen.
 * @param format format string
 * @param varg argument list
 * @return the number of characters printed
 * @see print
 */
int kprintv(const char *format, __builtin_va_list varg);

#endif

