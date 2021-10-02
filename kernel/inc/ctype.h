#ifndef CTYPE_H
#define CTYPE_H

static inline int isspace(int c) {
    return ((c == ' ')  ||
            (c == '\t') ||
            (c == '\v') ||
            (c == '\r') ||
            (c == '\n'));
}

static inline int isnum(int c) {
    return ((c >= '0') &&
            (c <= '9'));
}

static inline int isupper(int c) {
    return ((c >= 'A') &&
            (c <= 'Z'));
}

static inline int islower(int c) {
    return ((c >= 'a') &&
            (c <= 'z'));
}

static inline int isalpha(int c) {
    return (islower(c) || isupper(c));
}

static inline int isalnum(int c) {
    return (isnum(c) || isalpha(c));
}

static inline int tolower(int c) {
    if(islower(c)) { return c; }
    if(isupper(c)) { return c + ('a' - 'A'); }
    return 0;
}

#endif
