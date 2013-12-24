#include <time/time.h>
#include <err/error.h>
#include <types.h>

void rollover(u32);

struct time_block time_blocks[MAX_TIME_BLOCKS] = { [MAX_TIME_BLOCKS-1] = {&rollover, 0xFFFFFFFFFFFFFFFF, 0 } }; //!< Array of timeblocks used by various processes

u64 kerneltime = 0; //!< Number of elapsed ticks since the PIT was initialized

void rollover(u32 pid) //!< Called every 5,997,302.87 centuries
{
	kerror(ERR_LGERR, "Kernel time rolled over, a reboot is strongly suggested");
	kerror(ERR_INFO, "Congratulations, your computer stayed operational for 5,997,302.87 centuries!");
	add_time_block(&rollover, 0xFFFFFFFFFFFFFFFF, pid);
}

/**
 * \brief Called when count reaches 0.
 * This makes it easier for the timer interrupt to call event(). After calling event()
 * it resets the time_blocks[] entry so another process can use it.
 * @param n the entry within time_blocks
 * @see time_blocks
 */
void do_time_block_timeup(u32 n)
{
	u32 pid = time_blocks[n].pid;
	void (*event)(u32) = time_blocks[n].event;
	
	time_blocks[n].event = NULL;
	time_blocks[n].count = 0;
	time_blocks[n].pid   = 0;
	
	event(pid);
}

/**
 * \brief Adds a time block to time_blocks[].
 * Finds the first free time_block and sets its values to the ones supplied.
 * @param func the function to call when count reaches 0
 * @param count the number of ticks to wait before calling func()
 * @param pid the pid of the process that is using this time_block
 */
void add_time_block(void (*func)(u32), u64 count, u32 pid)
{
	int i = 0;
	for(; i < MAX_TIME_BLOCKS; i++)
	{
		if(time_blocks[i].event) continue;
		time_blocks[i].event = func;
		time_blocks[i].count = count;
		time_blocks[i].pid   = pid;
		return;
	}
	kerror(ERR_SMERR, "No free time blocks");
}