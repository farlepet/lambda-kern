#include <types.h>

#ifdef ARCH_X86
#include <kernel/arch/x86/intr/idt.h>
#include <kernel/arch/x86/intr/pit.h>
extern void exceptions_init(); //!< Initializes basic exception handlers. Found in intr/exceptions.asm
#endif

#ifdef ARCH_X86_64
#include <kernel/arch/x86_64/intr/idt.h>
#endif

/**
 * \brief Initializes interrupts.
 * Initializes based on the target architecture.
 */
void interrupts_init()
{
#ifdef ARCH_X86
	idt_init();
	exceptions_init();
__sti;
#endif
#ifdef ARCH_X86_64
	idt_init();
__sti;
#endif
}

/**
 * \brief Attaches an interrupt handler to an interrupt.
 * Attaches an interrupt handler to the specified interrupt.
 * @param n number of the interrupt
 * @param handler the location of the interrupt handler
 */
void set_interrupt(u32 n, void *handler)
{
#ifdef ARCH_X86
	set_idt(n, 0x08, 0x8E, handler);
#endif
#ifdef ARCH_X86_64
	set_idt(n, 0x08, 0x8E, handler);
#endif
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
}