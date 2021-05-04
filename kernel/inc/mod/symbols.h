#ifndef MOD_SYMBOLS_H
#define MOD_SYMBOLS_H

#include <types.h>

#include <mm/symbols.h>

/**
 * Find exported kernel symbol address by name
 * 
 * @param symbol Name of symbol to find address of
 * @param addr Address of symbol
 * 
 * @return 0 if symbol found and addr valid, else non-zero
 */
int module_symbol_find_kernel(const char *symbol, uintptr_t *addr);

int module_symbol_find_module(const char *symbol, uintptr_t *addr, const symbol_t *symbols);

#endif
