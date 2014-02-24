#include <string.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
u32 strlen(const char *str)
{
	register u32 i = 0;
	while(str[i]) i++;
	return i;
}

/**
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
u32 wcslen(const short *str)
{
	register u32 i = 0;
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
	while(*str1 && (*str1==*str2)) str1++, str2++;
	return *(const u8 *)str1 - *(const u8 *)str2;
}

int strncmp(const char *str1, const char *str2, u32 num)
{
	while(*str1 && (*str1 == *str2) && num--) str1++, str2++;
	return *(const u8 *)str1 - *(const u8 *)str2;
}

void *memcpy(void *dest, const void *src, u32 n)
{
	u8 *dp = dest;
	const u8 *sp = src;
	while (n--)
		*dp++ = *sp++;
	return dest;
}


void *memset(void *s, int c, u32 n)
{
	u8 *p = s;
	while(n--)
		*p++ = (u8)c;
	return s;
}
