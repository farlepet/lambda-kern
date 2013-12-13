#include <types.h>

#if defined(ARCH_X86) | defined(ARCH_X86_64)
#define TIMER_INT    0x20
#define KEYBOARD_INT 0x21
#endif // defined(ARCH_X86) | defined(ARCH_X86_64)

void interrupts_init();

void set_interrupt(u32 n, void *handler);