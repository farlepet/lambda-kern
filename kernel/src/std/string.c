#include <types.h>

/**
 * Calculates the length of a string.
 * 
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
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
int wcslen(const short *str)
{
	register int i = 0;
	while(str[i++]);
	return i;
}

/**
 * Checks to see if two strings are identical
 *
 * @param str1 first string
 * @param str2 second string
 */
 int strcmp(const char *str1, const char *str2)
 {
 	while(*str1 && (*str1==*str2))
        str1++,str2++;
    return *(const u8*)str1-*(const u8*)str2;
 }