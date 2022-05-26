#include <types.h>

/**
 * \brief Halts OS after printing error information.
 * 
 * Disabled interrupts, prints the provided error message, then halts the computer.
 * 
 * @param msg format string
 * @param ... arguments to go along with format string
 */
__noreturn
void _kpanic(char *msg, ...);

#define __kpanic_stringify1(X) #X
#define __kpanic_stringify2(X) __kpanic_stringify1(X)

#define kpanic(...) _kpanic(__FILE__ ":" __kpanic_stringify2(__LINE__) ": " __VA_ARGS__)

#define kassert(EXPR, ...) if(!(EXPR)) { kpanic(__VA_ARGS__); }

/* TODO: Move this declaration elsewhere */
void arch_kpanic_hook(void);
