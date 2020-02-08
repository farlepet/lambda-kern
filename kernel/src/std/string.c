#include <string.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
uint32_t strlen(const char *str)
{
	register uint32_t i = 0;
	while(str[i]) i++;
	return i;
}

/**
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
uint32_t wcslen(const short *str)
{
	register uint32_t i = 0;
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

int strncmp(const char *str1, const char *str2, uint32_t num)
{
	while(*str1 && (*str1 == *str2) && num--) str1++, str2++;
	return *(const u8 *)str1 - *(const u8 *)str2;
}


char *strchr(const char *s, int c) {
	while(*s) {
		if(*s == (char)c) return (char *)s;
		s++;
	}
	return NULL;
}


void *memcpy(void *dest, const void *src, uint32_t n)
{
	u8 *dp = dest;
	const u8 *sp = src;
	while (n--)
		*dp++ = *sp++;
	return dest;
}


void *memset(void *s, int c, uint32_t n)
{
	u8 *p = s;
	while(n--)
		*p++ = (u8)c;
	return s;
}
