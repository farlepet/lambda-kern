#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <lambda/export.h>
#include <data/cbuff.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <proc/atomic/lock.h>
#include <proc/atomic/tlock.h>
#include <proc/cond.h>
#include <proc/mtask.h>
#include <time/time.h>



int tlock_init(tlock_t *tlock) {
    memset(tlock, 0, sizeof(*tlock));

    if(cbuff_allocate(&tlock->threads, CONFIG_PROC_TLOCK_THREADS * sizeof(tlock_thread_t))) {
        return -1;
    }

    return 0;
}

void tlock_destroy(tlock_t *tlock) {
    cbuff_free(&tlock->threads);
}

/* TODO: Unlock next thread when a thread is killed */
void tlock_acquire(tlock_t *tlock) {
    if(lock_try(&tlock->lock)) {
        return;
    }

    /* TODO: Prevent race conditions */

    cond_t *cond = cond_create();
    if(!cond) {
        kpanic("Failure to create cond");
    }
    tlock_thread_t thread = {
        .cond = cond,
    };

    int ret = 1;
    while(ret) {
        lock(&tlock->thread_lock);
        int ret = cbuff_write((uint8_t *)&thread, sizeof(thread), &tlock->threads);
        if (ret && (ret != -ENOMEM)) {
            kpanic("Unexpected cbuff error adding thread to tlock: %08x", ret);
        }
        unlock(&tlock->thread_lock);
        run_sched();
    }

    cond_wait(cond);

    kfree(cond);
}

void tlock_release(tlock_t *tlock) {
    lock(&tlock->thread_lock);
    if(!tlock->threads.count) {
        /* No threads waiting, immediately unlock */
        unlock(&tlock->thread_lock);
        unlock(&tlock->lock);
        return;
    }

    /* Thread(s) waiting, queue up the next one. */
    tlock_thread_t thread;
    if(cbuff_read((uint8_t *)&thread, sizeof(thread), &tlock->threads)) {
        kpanic("Failure to read tlock thread data from cbuff");
    }
    unlock(&tlock->thread_lock);

    /* The next thread will assume the lock is already acquired, so do not
     * release it here */
    cond_signal(thread.cond);
}

