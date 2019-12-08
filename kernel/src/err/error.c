#include <proc/atomic.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <intr/int.h>
#include <config.h>
#include <types.h>
#include <video.h>

error_level minlvl = ERR_INFO; //!< Minimal level where messages are shown

lock_t kerror_lock = 0; //!< Only 1 message can be printed at a time
/**
 * \brief Prints information about the kernel.
 * Checks to see if the error level is >= the
 * minimum level, and if so, prints the current
 * clock tick, then prints the error message.
 * @param errlvl the severity of the message
 * @param msg the format string
 * @param ... the arguments to go along with the format string
 */
void kerror(error_level errlvl, char *msg, ...)
{

	if(errlvl >= minlvl)
	{
		if(interrupts_enabled()) lock_for(&kerror_lock, 8); // We don't want something like a kernel message from a lost task stopping us

		if(KERNEL_COLORCODE)
			kprintf("\e[31m[\e[32m%X%08X\e[31m]\e[0m ", (u32)(kerneltime >> 32), (u32)kerneltime);
		else
			kprintf("[%X%08X] ", (u32)(kerneltime >> 32), (u32)kerneltime);

		__builtin_va_list varg;
		__builtin_va_start(varg, msg);
		kprintv(msg, varg);
		__builtin_va_end(varg);
		kput('\n');
		
		if(interrupts_enabled()) unlock(&kerror_lock);
	}
}
