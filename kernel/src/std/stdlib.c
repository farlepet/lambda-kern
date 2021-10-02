#include <stdlib.h>
#include <ctype.h>

static inline int _conv_checkchar(char c, int base) {
    if(!isalnum(c)) { return 0; }
    if(isnum(c) && (c - '0') >= base) { return 0; }
    if(isalpha(c) && ((tolower(c) - 'a') >= (base - 10))) { return 0; }

    return 1;
}

static inline int _conv_convertchar(char c) {
    if(isnum(c))   { return (c - '0'); }
    if(isalpha(c)) { return ((tolower(c) - 'a') + 10); }

    return 1;
}

unsigned long strtoul(const char *__restrict ptr, char **__restrict endptr, int base) {
    if(base < 0 || base > 35) {
        return 0;
    }

    while(isspace(*ptr)) { ptr++; }
    if(ptr[0] == '0') {
        if((ptr[1] == 'x') ||
           (ptr[1] == 'X')) {
            if((base == 0) ||
               (base == 16)) {
                base = 16;
                ptr += 2;
            }
        } else if(base == 0) {
            base = 8;
        }
    }
    if(base == 0) {
        base = 10;
    }

    unsigned long val = 0;

    while(*ptr) {
        if(!_conv_checkchar(*ptr, base)) {
            break;
        }

        val *= base;
        val += _conv_convertchar(*ptr);

        ptr++;
    }

    if(endptr) { *endptr = (char *)ptr; }

    return val;
}
