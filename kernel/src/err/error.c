#include <proc/atomic.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <config.h>
#include <types.h>
#include <video.h>

error_level_e minlvl    = ERR_INFO; //!< Minimal level where messages are shown
uint32_t      debugmask = 0;

lock_t kerror_lock = 0; //!< Only 1 message can be printed at a time

void kerror(error_level_e errlvl, char *msg, ...) {
	if(errlvl >= minlvl) {
		if(interrupts_enabled()) lock_for(&kerror_lock, 8); // We don't want something like a kernel message from a lost task stopping us

		if(KERNEL_COLORCODE)
			kprintf("\e[31m[\e[32m%X%08X\e[31m]\e[0m ", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime);
		else
			kprintf("[%X%08X] ", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime);

		__builtin_va_list varg;
		__builtin_va_start(varg, msg);
		kprintv(msg, varg);
		__builtin_va_end(varg);
		kput('\n');
		
		if(interrupts_enabled()) unlock(&kerror_lock);
	}
}

void kdebug(debug_source_e src, char *msg, ...) {
	if(debugmask & (1UL << src)) {
		if(interrupts_enabled()) lock_for(&kerror_lock, 8); // We don't want something like a kernel message from a lost task stopping us

		if(KERNEL_COLORCODE)
			kprintf("\e[31m[\e[32m%X%08X\e[31m]\e[0m ", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime);
		else
			kprintf("[%X%08X] ", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime);

		__builtin_va_list varg;
		__builtin_va_start(varg, msg);
		kprintv(msg, varg);
		__builtin_va_end(varg);
		kput('\n');
		
		if(interrupts_enabled()) unlock(&kerror_lock);
	}
}
