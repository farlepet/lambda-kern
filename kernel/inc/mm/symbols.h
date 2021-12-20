#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <types.h>

extern uint32_t kern_start; // Start of the kernel
extern uint32_t kern_end; // Start of the kernel

typedef struct symbol {
    char    *name; //!< Symbol name
    uint32_t addr; //!< Start address of symbol
    uint32_t size; //!< Length of address space bounded by symbol
} symbol_t;

extern symbol_t sym_objects[];
extern symbol_t sym_functions[];

/**
 * @brief Find symbol within table given address
 * 
 * @param addr Address
 * @param symbols Symbol table in which to search
 * @return symbol_t* Symbol table entry, NULL if not found
 */
symbol_t *sym_find_object(uint32_t addr, symbol_t *symbols);

#endif
