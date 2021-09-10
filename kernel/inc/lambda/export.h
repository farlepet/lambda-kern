#ifndef LAMBDA_EXPORT_H
#define LAMBDA_EXPORT_H

#include <types.h>

#define LAMBDA_SYMBOLS_SECTION_NAME ".__lambda_symbols"
#define LAMBDA_STRINGS_SECTION_NAME ".__lambda_strings"

typedef struct {
    uintptr_t   addr;
    const char *name;
} lambda_symbol_t;

extern lambda_symbol_t __lambda_symbols_begin;
extern lambda_symbol_t __lambda_symbols_end;

extern char __lambda_strings_begin;
extern char __lambda_strings_end;

#define EXPORT_FUNC(func)                  \
    static const char                      \
    __attribute__((__used__, __section__(LAMBDA_STRINGS_SECTION_NAME))) \
    __lstr_func_##func[] = #func;          \
    static const lambda_symbol_t           \
    __attribute__((__used__, __section__(LAMBDA_SYMBOLS_SECTION_NAME))) \
    __lsym_func_##func = {                 \
        .addr = (uintptr_t)&func,          \
        .name = __lstr_func_##func         \
    }

#endif
