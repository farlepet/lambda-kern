#ifndef PROC_ATOMIC_TYPES_TLOCK_H
#define PROC_ATOMIC_TYPES_TLOCK_H

#include <stdatomic.h>

typedef struct tlock_struct tlock_t;

#include <data/types/cbuff.h>
#include <proc/types/cond.h>
#include <proc/atomic/types/lock.h>

/* TODO: Add to Kconfig */
#define CONFIG_PROC_TLOCK_THREADS 4


typedef struct tlock_thread_struct {
    /** Condition variable to notify thread */
    cond_t *cond;
} tlock_thread_t;

/**
 * @brief Thread-aware lock
 */
struct tlock_struct {
    /** Main lock */
    lock_t lock;
    /** Lock for accessing threads field */
    lock_t thread_lock;
    /* TODO: Should this use a cbuff or a llist? */
    /** List of threads that are attempting to acquire this lock */
    cbuff_t threads;
};

#define STATIC_TLOCK() { .lock = 0, .thread_lock = 0, .threads = STATIC_CBUFF(CONFIG_PROC_TLOCK_THREADS * sizeof(tlock_thread_t))}

#endif

