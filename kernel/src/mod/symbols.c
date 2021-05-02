#include <lambda/export.h>
#include <mod/symbols.h>
#include <err/error.h>

#include <string.h>

static int _module_symbol_find_kernel(const char *symbol, uintptr_t *addr) {
    lambda_symbol_t *sym = &__lambda_symbols_begin;
    while(sym < &__lambda_symbols_end) {
        if(!strcmp(sym->name, symbol)) {
            *addr = sym->addr;
            return 0;
        }
        sym++;
    }

    return -1;
}

static int _module_symbol_find_modules(const char *symbol, uintptr_t *addr) {
    (void)symbol;
    (void)addr;

    /* TODO */

    return -1;
}

int module_symbol_find(const char *symbol, uintptr_t *addr) {
    if(addr == NULL) {
        return -1;
    }
    
    if(!_module_symbol_find_kernel(symbol, addr)) {
        return 0;
    }
    if(!_module_symbol_find_modules(symbol, addr)) {
        return 0;
    }

    return -1;
}