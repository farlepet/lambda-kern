#include <types.h>
#include <err/error.h>

#if  defined(ARCH_X86)

#include <intr/idt.h>
#include <intr/pit.h>
#include <intr/int.h>
extern void exceptions_init(); //!< Initializes basic exception handlers. Found in intr/exceptions.asm

#elif defined(ARCH_X86_64)

#include <intr/idt.h>
#include <intr/int.h>

#endif

/**
 * \brief Initializes interrupts.
 * Initializes based on the target architecture.
 */
void interrupts_init()
{
#if  defined(ARCH_X86)
	idt_init();
	exceptions_init();
	enable_interrupts();
#endif
	kerror(ERR_BOOTINFO, "Interrupts enabled");
}

/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(u32 n, void *handler)
{
#if   defined(ARCH_X86)
	set_idt(n, 0x08, 0x8E, handler);
#endif
	kerror(ERR_INFO, "Interrupt vector 0x%02X set", n);
}

/**
 * \brief Initializes the system timer.
 * Initializes the timer used by the target architecture.
 * @param quantum the speed in Hz
 */
void timer_init(u32 quantum)
{
#ifdef ARCH_X86
	pit_init(quantum);
#else
	(void)quantum;
#endif
	kerror(ERR_BOOTINFO, "Timer initialized");
}