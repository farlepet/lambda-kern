#ifndef TIME_H
#define TIME_H

#include <types.h>

u64 kerneltime; //!< Number of elapsed ticks since the PIT was initialized

/**
 * Used when specifying UNIX time (not timer ticks).
 *   - It is signed so it can represent time before January 1, 1970
 */
typedef s64 time_t; //!< Typedef used when specifying UNIX time (not timer ticks)

/**
 * \brief A structure to help with timing.
 * A structure that helps processes keep time. Every timer tick, count will
 * be decremented by 1. When count reaches 0, event() is called.
 * @see do_time_block_timeup
 */
struct time_block
{
	void (*event)(int pid); //!< Called when `count` = 0
	u64  count;             //!< The number of ticks left
	int  pid;               //!< PID of the process using this block
};

#define MAX_TIME_BLOCKS 64 //!< Maximum number of timer blocks able to be used. We cannot let this get too high, or we will experience slowdown.

struct time_block time_blocks[MAX_TIME_BLOCKS]; //!< Array of timeblocks used by various processes

/**
 * \brief Called when count reaches 0.
 * This makes it easier for the timer interrupt to call event(). After calling event()
 * it resets the time_blocks[] entry so another process can use it.
 * @param n the entry within time_blocks
 * @see time_blocks
 */
void do_time_block_timeup(u32 n);

/**
 * \brief Adds a time block to time_blocks[].
 * Finds the first free time_block and sets its values to the ones supplied.
 * @param func the function to call when count reaches 0
 * @param count the number of ticks to wait before calling func()
 * @param pid the pid of the process that is using this time_block
 */
void add_time_block(void (*func)(int), u64 count, int pid);

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of ticks to wait for
 */
void delay(u64 delay);

#endif
