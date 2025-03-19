#include <errno.h>
#include <string.h>

#include <lambda/export.h>
#include <mod/symbols.h>
#include <err/error.h>


int module_symbol_find_kernel(const char *symbol, uintptr_t *addr) {
    lambda_symbol_t *sym = &__lambda_symbols_begin;
    while(sym < &__lambda_symbols_end) {
        if(!strcmp(sym->name, symbol)) {
            *addr = sym->addr;
            return 0;
        }
        sym++;
    }

    return -EUNSPEC;
}

int module_symbol_find_module(const char *symbol, uintptr_t *addr, const symbol_t *symbols) {
    (void)symbol;
    (void)addr;

    const symbol_t *sym = symbols;
    while(sym->addr != 0xFFFFFFFF) {
        if(!strcmp(symbol, sym->name)) {
            *addr = sym->addr;
            return 0;
        }
        sym++;
    }

    return -EUNSPEC;
}
