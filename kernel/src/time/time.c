#include <stddef.h>
#include <stdint.h>

#include <err/error.h>
#include <err/panic.h>
#include <time/time.h>

void rollover(int);

static time_block_t _time_blocks[MAX_TIME_BLOCKS] = { 0 }; //!< Array of timeblocks used by various processes

uint64_t kerneltime = 0;

static void _do_time_block_timeup(uint32_t n) {
    void (*event)(void *) = _time_blocks[n].event;
    void  *data           = _time_blocks[n].data;

    _time_blocks[n].event = NULL;
    _time_blocks[n].end   = 0;
    _time_blocks[n].tid   = 0;
    
    event(data);
}

void add_time_block(void (*func)(void *), void *data, uint64_t off, int tid) {
    int i = 0;
    for(; i < MAX_TIME_BLOCKS; i++)
    {
        if(_time_blocks[i].event) continue;
        /* TODO: Add a mutex lock here */
        _time_blocks[i].event = func;
        _time_blocks[i].data  = data;
        _time_blocks[i].end   = (kerneltime + off);
        _time_blocks[i].tid   = tid;
        return;
    }
    kpanic("No free time blocks");
}

__hot
void time_update(uint64_t off) {
    kerneltime += off;

    for(uint32_t i = 0; i < MAX_TIME_BLOCKS; i++) {
        if(_time_blocks[i].event) {
            if(kerneltime >= _time_blocks[i].end) {
                _do_time_block_timeup(i);
            }
        }
    }
}
