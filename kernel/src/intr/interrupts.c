#include <intr/intr.h>
#include <err/error.h>
#include <types.h>

#if  defined(ARCH_X86)

#include <intr/idt.h>
#include <intr/pit.h>
#include <intr/int.h>
#include <mm/gdt.h>
extern void exceptions_init(); //!< Initializes basic exception handlers. Found in intr/exceptions.asm

#endif

/**
 * \brief Initializes interrupts.
 * Initializes based on the target architecture.
 */
void interrupts_init()
{
	kerror(ERR_BOOTINFO, "Enabling Interrupts");
#if  defined(ARCH_X86)
	kerror(ERR_BOOTINFO, "  -> GDT");
	gdt_init();
	kerror(ERR_BOOTINFO, "  -> IDT");
	idt_init();
	kerror(ERR_BOOTINFO, "  -> Exceptions");
	exceptions_init();
	kerror(ERR_BOOTINFO, "  -> STI");
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
	set_idt((u8)n, 0x08, 0x8E, handler);
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
