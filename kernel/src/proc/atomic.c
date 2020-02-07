#include <proc/atomic.h>
#include <proc/mtask.h>

void lock(lock_t *lock)
{
    int old = 0;
    while(!a_cmp_chx_weak(lock, &old, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
    {
        run_sched();
        old = 0;
    }
}

int lock_for(lock_t *lock, u32 ticks)
{
    int old = 0;
    u64 endticks = kerneltime + ticks;
	
    while(!a_cmp_chx_weak(lock, &old, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
    {
        run_sched();
        old = 0;
        if(kerneltime >= endticks) return 1;
    }
    return 0;
}