#include <stddef.h>

#include <mm/symbols.h>

/* TODO: Load symbols from file */
symbol_t sym_objects[]   = { { "EOS", 0xFFFFFFFF, 0x0 } };
symbol_t sym_functions[] = { { "EOS", 0xFFFFFFFF, 0x0 } };

symbol_t *sym_find_object(uint32_t addr, symbol_t *symbols) {
    if(symbols == NULL) symbols = sym_objects;
    addr -= (uint32_t)&kern_start;
    int i = 0;
    symbol_t *obj = NULL;
    while((obj = &symbols[i])->addr != 0xFFFFFFFF) {
        if(obj->addr <= addr) {
            if((obj->addr + obj->size) > addr) {
                return obj;
            }
        }
        i++;
    }
    return NULL;
}

