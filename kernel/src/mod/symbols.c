#include <lambda/export.h>
#include <mod/symbols.h>

#include <string.h>

int module_symbol_find(const char *symbol, uintptr_t *addr) {
    if(addr == NULL) {
        return -1;
    }
    
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