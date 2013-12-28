#include <types.h>
#include <video.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <intr/int.h>

error_level minlvl = ERR_BOOTINFO; //!< Minimal level where messages are shown

/**
 * \brief Prints information about the kernel.
 * Disables interrupts so it will not get interrupted, checks to see if the 
 * error level is >= the minimum level, and if so, prints the current
 * clock tick, then prints the error message. Then, if en_int != 0, it
 * enables interrupts again.
 * @param errlvl the severity of the message
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kerror(error_level errlvl, char *msg, ...)
{
	int en_int = interrupts_enabled();
	disable_interrupts();
	
	if(errlvl >= minlvl)
	{
		kprintf("[%X%08X] ", (u32)(kerneltime >> 32), (u32)kerneltime);
		__builtin_va_list varg;
		__builtin_va_start(varg, msg);
		kprintv(msg, varg);
		__builtin_va_end(varg);
		kput('\n');
	}
	
	if(en_int) enable_interrupts();
}