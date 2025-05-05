#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
size_t strlen(const char *str);

/**
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
size_t wcslen(const short *str);

/**
 * Checks to see if two strings are identical
 *
 * @param str1 first string
 * @param str2 second string
 */
int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

char *strchr(const char *s, int c);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *s, int c, size_t n);

void *memmove(void *dst, const void *src, size_t n);

/**
 * @brief Compare two sets of data for equality
 *
 * @param s1 Pointer to first set of data
 * @param s2 Pointer to second set of data
 * @param n Size of data to compare, in bytes
 *
 * @return 0 if both sets of data are equal, else non-zero
 */
int memcmp(const void *s1, const void *s2, size_t n);

#endif

