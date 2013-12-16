#include <time/time.h>
#include <types.h>

struct time_block time_blocks[MAX_TIME_BLOCKS] = { { NULL, 0, 0 }, };

void do_time_block_timeup(u32 n)
{
	time_blocks[n].event(time_blocks[n].pid);

	time_blocks[n].event = NULL;
	time_blocks[n].count = 0;
	time_blocks[n].pid   = 0;
}

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
	// Produce error here
}