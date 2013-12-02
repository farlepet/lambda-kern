#ifndef VGA_PRINT_H
#define VGA_PRINT_H

#include <types.h>

void vga_clear();
void vga_put(u8);
void vga_print(char *);

#endif