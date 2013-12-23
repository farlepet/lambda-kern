#include <time/time.h>
#include <proc/mtask.h>
#include <intr/int.h>

u8 timeup[256]; //!< Table of values corresponding to pid's telling if the timer is done yet

/**
 * \brief Tells `delay` it can return.
 * Called when the time specified by `delay` has run out. Sets the value in `timeup`
 * corresponding to the callers pid to 1.
 * @param pid the pid of the caller to `delay`
 */
void time_over(u32 pid)
{
	timeup[pid] = 1;
}

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of ticks to wait for
 */
void delay(u64 delay)
{
	u32 pid = current_pid;
	timeup[pid] = 0;
	add_time_block(&time_over, delay, pid);
	while(!timeup[pid]) busy_wait();
}

