#include <mm/symbols.h>

symbol_t *sym_find_object(u32 addr, symbol_t *symbols) {
    if(symbols == NULL) symbols = sym_objects;
    addr -= (u32)&kern_start;
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
