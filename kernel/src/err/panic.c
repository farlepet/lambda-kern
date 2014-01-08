#include <intr/intr.h>
#include <err/panic.h>
#include <types.h>
#include <video.h>

/**
 * \brief Halts OS after printing error information.
 * Disabled interrupts, prints the provided error message, then halts the computer.
 * @param msg format string
 * @param ... arguments to go along with format string
 */
void __noreturn kpanic(char *msg, ...)
{
	disable_interrupts();
	
	__builtin_va_list varg;
	__builtin_va_start(varg, msg);
	
	kprintf("Kernel panic:\n    ");
	kprintv(msg, varg);
	
	__builtin_va_end(varg);
	
	// Print regs here
	
	
	for(;;) interrupt_halt();
}