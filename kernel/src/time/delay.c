#include <proc/mtask.h>
#include <time/time.h>
#include <intr/int.h>

u8 timeup[500]; //!< Table of values corresponding to pid's telling if the timer is done yet

/**
 * \brief Tells `delay` it can return.
 * Called when the time specified by `delay` has run out. Sets the value in `timeup`
 * corresponding to the callers pid to 1.
 * @param pid the pid of the caller to `delay`
 */
void time_over(int pid)
{
	int idx = proc_by_pid(pid);

	procs[idx].blocked &= ~BLOCK_DELAY;
}

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of ticks to wait for
 */
void delay(u64 delay)
{
	int pid = current_pid;
	int idx = proc_by_pid(pid);
	add_time_block(&time_over, delay, pid);
	procs[idx].blocked |= BLOCK_DELAY;
	interrupt_halt(); // Halt until multitasking comes in
}

