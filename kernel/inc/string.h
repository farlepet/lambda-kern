#ifndef STRING_H
#define STRING_H

#include <types.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
u32 strlen(const char *str);

/**
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
u32 wcslen(const short *str);

/**
 * Checks to see if two strings are identical
 *
 * @param str1 first string
 * @param str2 second string
 */
int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, u32 num);

char *strchr(const char *s, int c);

void *memcpy(void *dest, const void *src, u32 n);

void *memset(void *s, int c, u32 n);

#endif
