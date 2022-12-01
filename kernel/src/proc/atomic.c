#include <stdbool.h>
#include <stdint.h>

#include <lambda/export.h>
#include <proc/atomic.h>
#include <proc/mtask.h>

void lock(lock_t *lock) {
    int test = 1;

    while(atomic_exchange_explicit(lock, test, memory_order_acquire)) {
        run_sched();
    }
}
EXPORT_FUNC(lock);

int lock_for(lock_t *lock, uint32_t ms) {
    int test = 1;
    uint64_t end = kerneltime + ms;
    
    while(atomic_exchange_explicit(lock, test, memory_order_acquire)) {
        if(kerneltime >= end) return 1;
        run_sched();
    }
    
    return 0;
}
EXPORT_FUNC(lock_for);

