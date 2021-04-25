#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <time/time.h>
#include <err/error.h>

uint8_t timeup[500]; //!< Table of values corresponding to pid's telling if the timer is done yet

/**
 * \brief Tells `delay` it can return.
 * Called when the time specified by `delay` has run out. Sets the value in `timeup`
 * corresponding to the callers tid to 1.
 * @param tid the tid of the caller to `delay`
 */
static void time_over(int tid) {
	kthread_t *thread = thread_by_tid(tid);

	thread->blocked &= (uint32_t)~BLOCK_DELAY;
}

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of ticks to wait for
 */
void delay(uint64_t delay) {
	add_time_block(&time_over, delay, curr_proc->threads[curr_thread].tid);
	curr_proc->threads[curr_thread].blocked |= BLOCK_DELAY;

	while(curr_proc->threads[curr_thread].blocked & BLOCK_DELAY) {
		interrupt_halt(); // Halt until multitasking comes in
	}
}

