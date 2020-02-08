#include <types.h>

/**
 * \brief Halts OS after printing error information.
 * 
 * Disabled interrupts, prints the provided error message, then halts the computer.
 * 
 * @param msg format string
 * @param ... arguments to go along with format string
 */
void kpanic(char *msg, ...);