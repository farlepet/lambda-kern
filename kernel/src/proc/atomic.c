#include <lambda/export.h>
#include <proc/atomic.h>
#include <proc/mtask.h>

void lock(lock_t *lock) {
    int old = 0;
    while(!a_cmp_chx_weak(lock, &old, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        run_sched();
        old = 0;
    }
}
EXPORT_FUNC(lock);

int lock_for(lock_t *lock, uint32_t ms) {
    int old = 0;
    uint64_t end = kerneltime + ms;
    
    while(!a_cmp_chx_weak(lock, &old, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        run_sched();
        old = 0;
        if(kerneltime >= end) return 1;
    }
    return 0;
}
EXPORT_FUNC(lock_for);
