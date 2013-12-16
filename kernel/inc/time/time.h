#ifndef TIME_H
#define TIME_H

#include <types.h>


struct time_block
{
	void (*event)(u32 pid); //!< Called when `count` = 0
	u64  count;             //!< The number of ticks left
	u32  pid;               //!< PID of the process using this block
};

#define MAX_TIME_BLOCKS 64

struct time_block time_blocks[MAX_TIME_BLOCKS];


void do_time_block_timeup(u32 n);

#endif