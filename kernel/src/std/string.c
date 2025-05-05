#include <string.h>

#include <lambda/config_defs.h>
#include <lambda/export.h>
#include <err/panic.h>
#include <mm/mm.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
size_t strlen(const char *str) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(str)) {
        kpanic("Bad address: %p", str);
    }
#endif
    register size_t i = 0;
    while(str[i]) { i++; }
    return i;
}
EXPORT_FUNC(strlen);

/**
 * Calculates the length of a wide string.
 * 
 * @param str the wide string to calculate the length of
 * @return the length of the wide string
 */
size_t wcslen(const short *str) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(str)) {
        kpanic("Bad address: %p", str);
    }
#endif
    register size_t i = 0;
    while(str[i]) { i++; }
    return i;
}
EXPORT_FUNC(wcslen);

/**
 * Checks to see if two strings are identical
 *
 * @param str1 first string
 * @param str2 second string
 */
int strcmp(const char *str1, const char *str2) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(str1)) {
        kpanic("Bad str1 address: %p", str1);
    }
    if(!mm_check_addr(str2)) {
        kpanic("Bad str2 address: %p", str2);
    }
#endif
    while(*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const uint8_t *)str1 - *(const uint8_t *)str2;
}
EXPORT_FUNC(strcmp);

int strncmp(const char *str1, const char *str2, size_t num) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(str1)) {
        kpanic("Bad str1 address: %p", str1);
    }
    if(!mm_check_addr(str2)) {
        kpanic("Bad str2 address: %p", str2);
    }
#endif
    while(*str1 && (*str1 == *str2) && --num) {
        str1++;
        str2++;
    }
    return *(const uint8_t *)str1 - *(const uint8_t *)str2;
}
EXPORT_FUNC(strncmp);

char *strchr(const char *s, int c) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(s)) {
        kpanic("Bad address: %p", s);
    }
#endif
    while(*s) {
        if(*s == (char)c) return (char *)s;
        s++;
    }
    return NULL;
}
EXPORT_FUNC(strchr);

char *strcpy(char *dest, const char *src) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(dest)) {
        kpanic("Bad destination address: %p", dest);
    }
    if(!mm_check_addr(src)) {
        kpanic("Bad source address: %p", src);
    }
#endif
    size_t i = 0;
    while(src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    
    return dest;
}
EXPORT_FUNC(strcpy);

char *strncpy(char *dest, const char *src, size_t n) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(dest)) {
        kpanic("Bad destination address: %p", dest);
    }
    if(!mm_check_addr(src)) {
        kpanic("Bad source address: %p", src);
    }
#endif
    size_t i = 0;
    while((i < n) && src[i]) {
        dest[i] = src[i];
        i++;
    }
    while(i < n) {
        dest[i++] = '\0';
    }
    
    return dest;
}
EXPORT_FUNC(strncpy);

void *memcpy(void *dest, const void *src, size_t n) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(dest)) {
        kpanic("Bad destination address: %p", dest);
    }
    if(!mm_check_addr(src)) {
        kpanic("Bad source address: %p", src);
    }
#endif
    uint8_t *dp = dest;
    const uint8_t *sp = src;
    while (n--) {
        *dp++ = *sp++;
    }
    return dest;
}
EXPORT_FUNC(memcpy);


void *memset(void *s, int c, size_t n) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(s)) {
        kpanic("Bad address: %p", s);
    }
#endif
    uint8_t *p = s;
    while(n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}
EXPORT_FUNC(memset);


void *memmove(void *dst, const void *src, size_t n) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(dst)) {
        kpanic("Bad destination address: %p", dst);
    }
    if(!mm_check_addr(src)) {
        kpanic("Bad source address: %p", src);
    }
#endif
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
EXPORT_FUNC(memmove);

int memcmp(const void *s1, const void *s2, size_t n) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    if(!mm_check_addr(s1)) {
        kpanic("Bad s1 address: %p", s1);
    }
    if(!mm_check_addr(s2)) {
        kpanic("Bad s2 address: %p", s2);
    }
#endif
    while (n--) {
        if (((const uint8_t *)s1)[n] != ((const uint8_t *)s2)[n]) {
            return ((const uint8_t *)s1)[n] - ((const uint8_t *)s2)[n];
        }
    }
    return 0;
}
EXPORT_FUNC(memcmp);
