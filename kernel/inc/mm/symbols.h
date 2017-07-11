#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <types.h>

extern u32 kern_start; // Start of the kernel

typedef struct symbol {
    char *name;
    u32 addr;
    u32 size;
} symbol_t;

extern symbol_t sym_objects[];
extern symbol_t sym_functions[];

symbol_t *sym_find_object(u32 addr);

#endif
