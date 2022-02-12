#include <lambda/export.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <time/time.h>
#include <err/error.h>
#include <err/panic.h>

/**
 * \brief Tells `delay` it can return.
 * Called when the time specified by `delay` has run out. Sets the value in `timeup`
 * corresponding to the callers tid to 1.
 * @param tid the tid of the caller to `delay`
 */
static void time_over(int tid) {
    kthread_t *thread = thread_by_tid(tid);
    if(thread == NULL) {
        kpanic("thread is NULL for TID %d!", tid);
    }

    thread->blocked &= (uint32_t)~BLOCK_DELAY;
}

void delay(uint32_t delay) {
    kthread_t *curr_thread = mtask_get_curr_thread();

    add_time_block(&time_over, (uint64_t)delay * 1000000ULL, curr_thread->tid);
    curr_thread->blocked |= BLOCK_DELAY;

    while(curr_thread->blocked & BLOCK_DELAY) {
        interrupt_halt(); // Halt until multitasking comes in
    }
}
EXPORT_FUNC(delay);
