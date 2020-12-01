#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <time/time.h>
#include <err/error.h>

uint8_t timeup[500]; //!< Table of values corresponding to pid's telling if the timer is done yet

/**
 * \brief Tells `delay` it can return.
 * Called when the time specified by `delay` has run out. Sets the value in `timeup`
 * corresponding to the callers pid to 1.
 * @param pid the pid of the caller to `delay`
 */
static void time_over(int pid) {
	struct kproc *proc = proc_by_pid(pid);

	kerror(ERR_BOOTINFO, "time_over(%d)", pid);

	proc->blocked &= (uint32_t)~BLOCK_DELAY;
}

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of ticks to wait for
 */
void delay(uint64_t delay) {
	add_time_block(&time_over, delay, curr_proc->pid);
	curr_proc->blocked |= BLOCK_DELAY;
	kerror(ERR_BOOTINFO, "delay(%d, %d)", curr_proc->pid, delay);
	interrupt_halt(); // Halt until multitasking comes in
}

