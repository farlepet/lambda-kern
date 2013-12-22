#include <types.h>
#include <video.h>
#include <intr/intr.h>

/**
 * \brief Halts OS after printing error information.
 * Disabled interrupts, prints the provided error message, then halts the computer.
 * @param msg format string
 * @param ... arguments to go along with format string
 */
void kpanic(char *msg, ...)
{
	ptr_t *varg = (ptr_t *)&msg;
	
	disable_interrupts();
	
	kprintf("Kernel panic:\n    ");
	kprintv(msg, ++varg);
	
	// Print regs here
	
	interrupt_halt();
}