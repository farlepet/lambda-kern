#ifndef VGA_PRINT_H
#define VGA_PRINT_H

#include <types.h>

// Clear the VGA screen
void vga_clear();

// Process and possible print a character
void vga_put(u8);

// Process and print a string of characters
void vga_print(char *);

// Print an unsigned number using a specified base between 1 and 16, inclusive
void vga_printnum(int, int);

#endif