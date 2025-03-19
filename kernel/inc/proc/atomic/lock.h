#ifndef PROC_ATOMIC_LOCK_H
#define PROC_ATOMIC_LOCK_H

#include <errno.h>
#include <stdint.h>

#include <proc/atomic/types/lock.h>

/**
 * \brief Release an atomic lock
 * 
 * @param lock Lock to release
 */
static inline void unlock(lock_t *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
}

/**
 * \brief Aquire an atomic lock
 * 
 * @param lock Lock to aquire
 */
void lock(lock_t *lock);

/**
 * \brief Aquire atomic lock, with timeout
 * 
 * @param lock Lock to attempt to aquire
 * @param ticks How long to attempt to aquire lock, in milliseconds
 * @return int 0 if successful, else -ETIMEDOUT
 */
int lock_for(lock_t *lock, uint32_t ms);

/**
 * \brief Attempt to acquire lock without blocking
 * 
 * @param lock Lock to acquire
 * @return 0 on success, -EUNSPEC on failure
 */
static inline int lock_try(lock_t *lock) {
    int test = 1;
    return (atomic_exchange_explicit(lock, test, memory_order_acquire) ? 0 : -EUNSPEC);
}

#endif

