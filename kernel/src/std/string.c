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
	return *(const uint8_t *)str1 - *(const uint8_t *)str2;
}

int strncmp(const char *str1, const char *str2, uint32_t num)
{
	while(*str1 && (*str1 == *str2) && num--) str1++, str2++;
	return *(const uint8_t *)str1 - *(const uint8_t *)str2;
}

char *strchr(const char *s, int c) {
	while(*s) {
		if(*s == (char)c) return (char *)s;
		s++;
	}
	return NULL;
}

char *strcpy(char *dest, const char *src) {
	size_t i = 0;
	while(src[i]) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
	size_t i = 0;
	while(src[i] && (i < n)) {
		dest[i] = src[i];
		i++;
	}
	while(i < n) {
		dest[i++] = '\0';
	}
	
	return dest;
}

void *memcpy(void *dest, const void *src, uint32_t n)
{
	uint8_t *dp = dest;
	const uint8_t *sp = src;
	while (n--)
		*dp++ = *sp++;
	return dest;
}


void *memset(void *s, int c, uint32_t n)
{
	uint8_t *p = s;
	while(n--)
		*p++ = (uint8_t)c;
	return s;
}


void *memmove(void *dst, const void *src, uint32_t n) {
	const uint8_t *_src = src;
	uint8_t *_dst = dst;

	if(dst < src) {
		while(n--) {
			*_dst++ = *_src++;
		}
	} else {
		_dst += n;
		_src += n;

		while(n--) {
			*--_dst = *--_src;
		}
	}

	return dst;
}
