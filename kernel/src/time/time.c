#include <time/time.h>
#include <err/error.h>
#include <types.h>

void rollover(int);

static time_block_t _time_blocks[MAX_TIME_BLOCKS] = { [MAX_TIME_BLOCKS-1] = {&rollover, 0xFFFFFFFFFFFFFFFF, 0 } }; //!< Array of timeblocks used by various processes

uint64_t kerneltime = 0; //!< Number of elapsed ticks since the PIT was initialized

void rollover(int pid) //!< Called when the timer rolls over
{
    kerror(ERR_CRIT, "Kernel time rolled over, a reboot is strongly suggested");
    add_time_block(&rollover, 0xFFFFFFFFFFFFFFFF, pid);
}


void do_time_block_timeup(uint32_t n)
{
    int pid = _time_blocks[n].pid;
    void (*event)(int) = _time_blocks[n].event;
    
    _time_blocks[n].event = NULL;
    _time_blocks[n].count = 0;
    _time_blocks[n].pid   = 0;
    
    event(pid);
}

void add_time_block(void (*func)(int), uint64_t count, int pid)
{
    int i = 0;
    for(; i < MAX_TIME_BLOCKS; i++)
    {
        if(_time_blocks[i].event) continue;
        _time_blocks[i].event = func;
        _time_blocks[i].count = count;
        _time_blocks[i].pid   = pid;
        return;
    }
    kerror(ERR_ERROR, "No free time blocks");
}

__hot
void time_update(uint64_t off) {
    kerneltime += off;

    /* TODO: Base time blocks off real-world time, rather than kernel ticks. */
	for(uint32_t i = 0; i < MAX_TIME_BLOCKS; i++) {
		if(_time_blocks[i].event) {
			if(--_time_blocks[i].count == 0x00000000) {
				do_time_block_timeup(i);
			}
		}
	}
}
