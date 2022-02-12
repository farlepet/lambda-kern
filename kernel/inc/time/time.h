#ifndef TIME_H
#define TIME_H

#include <types.h>

/**
 * @brief Nanoseconds since system timer initialization
 * 
 * @note 2^64 nanoseconds = ~585 years
 */

extern uint64_t kerneltime;

/**
 * Used when specifying UNIX time (not timer ticks).
 *   - It is signed so it can represent time before January 1, 1970
 */
typedef int64_t time_t; //!< Typedef used when specifying UNIX time (not timer ticks)

/**
 * \brief A structure to help with timing.
 * A structure that helps processes keep time. Every timer tick, count will
 * be decremented by 1. When count reaches 0, event() is called.
 * @see do_time_block_timeup
 */
typedef struct time_block
{
	void     (*event)(int pid); //!< Called when `count` = 0
	uint64_t end;               //!< Value of kerneltime at which timer expires
	int      pid;               //!< PID of the process using this block
} time_block_t;

#define MAX_TIME_BLOCKS 64 //!< Maximum number of timer blocks able to be used. We cannot let this get too high, or we will experience slowdown.

/**
 * \brief Adds a time block to time_blocks[].
 * Finds the first free time_block and sets its values to the ones supplied.
 * @param func the function to call when count reaches 0
 * @param off the number of nanoseconds to wait before calling func()
 * @param pid the pid of the process that is using this time_block
 */
void add_time_block(void (*func)(int), uint64_t off, int pid);

/**
 * \brief Waits for a specified amount of time.
 * Creates a time block to wait for `delay` clock ticks, then waits until the
 * time has run out.
 * @param delay number of milliseconds to wait for
 */
void delay(uint32_t delay);

/**
 * \brief Updates kernel time and time blocks.
 * 
 * @param off Number of nanoseconds since last call
 */
void time_update(uint64_t off);

#endif
