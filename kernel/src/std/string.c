#include <types.h>

/**
 * \brief Calculates the length of a string.
 * Calculates the length of a string.
 * @param str the string to calculate the length of
 * @return the length of the string
 */
int strlen(const char *str)
{
	register int i = 0;
	while(str[i++]);
	return i;
}

/**
 * \brief Calculates the length of a wide string.
 * Calculates the length of a wide string.
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
int wcslen(const short *str)
{
	register int i = 0;
	while(str[i++]);
	return i;
}