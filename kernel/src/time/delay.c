#include <lambda/export.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <proc/cond.h>
#include <time/time.h>
#include <err/panic.h>
#include <mm/alloc.h>

static void _time_over(void *cond) {
    cond_signal(cond);
}

void delay(uint32_t delay) {
    kthread_t *curr_thread = mtask_get_curr_thread();

    cond_t *cond = kmalloc(sizeof(cond_t));
    if(cond == NULL) {
        kpanic("Could not allocate conditional!");
    }
    cond_init(cond);

    disable_interrupts();
    add_time_block(&_time_over, cond, (uint64_t)delay * 1000000ULL, curr_thread->tid);
    cond_wait(cond);

    kfree(cond);
}
EXPORT_FUNC(delay);
