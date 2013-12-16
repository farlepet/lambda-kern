#ifndef TIME_H
#define TIME_H

#include <types.h>

/**
 * \brief A structure to help with timing.
 * A structure that helps processes keep time. Every timer tick, count will
 * be decremented by 1. When count reaches 0, event() is called.
 * @see do_time_block_timeup
 */
struct time_block
{
	void (*event)(u32 pid); //!< Called when `count` = 0
	u64  count;             //!< The number of ticks left
	u32  pid;               //!< PID of the process using this block
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
void add_time_block(void (*func)(u32), u64 count, u32 pid);

#endif