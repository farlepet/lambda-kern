#ifndef PROC_ATOMIC_TLOCK_H
#define PROC_ATOMIC_TLOCK_H

#include <proc/atomic/types/tlock.h>

/**
 * @brief Initialize and acquire memory for a thread-aware lock
 *
 * @param tlock Lock to initialize
 *
 * @return 0 on success, else < 0
 */
int tlock_init(tlock_t *tlock);

/**
 * @brief Destroy and free a thread-aware lock
 *
 * @param tlock Lock to destroy
 */
void tlock_destroy(tlock_t *tlock);

/**
 * @brief Acquire a thread-aware lock
 *
 * @param tlock Lock to acquire
 */
void tlock_acquire(tlock_t *tlock);

/**
 * @brief Release a thread-aware lock, signaling the next awating threade (if
 * applicable) to acquire it.
 *
 * @param tlock Lock to release
 */
void tlock_release(tlock_t *tlock);

#endif

