#include <types.h>
#include <video.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>

error_level minlvl = ERR_BOOTINFO; //!< Minimal level where messages are shown

/**
 * \brief Prints information about the kernel.
 * Disables interrupts so it will not get interrupted, checks to see if the 
 * error level is >= the minimum level, and if so, prints the current
 * clock tick, then prints the error message. Then, if en_int != 0, it
 * enables interrupts again.
 * @param errlvl the severity of the message
 * @param en_int wether or not to enable interrupts when its done
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kerror(error_level errlvl, int en_int, char *msg, ...)
{
	disable_interrupts();
	
	if(errlvl >= minlvl)
	{
		ptr_t *varg = (ptr_t *)&msg;
		kprintf("[%08X%08X] ", (u32)(kerneltime >> 32), (u32)kerneltime);
		kprintv(msg, ++varg);
		kput('\n');
	}
	
	if(en_int) enable_interrupts();
}