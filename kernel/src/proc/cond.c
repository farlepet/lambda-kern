#include <errno.h>
#include <string.h>

#include <lambda/config_defs.h>
#include <lambda/export.h>
#include <proc/atomic/lock.h>
#include <proc/cond.h>
#include <proc/mtask.h>
#include <mm/alloc.h>

int cond_init(cond_t *cond) {
    memset(cond, 0, sizeof(cond_t));
    llist_init(&cond->list);

    return 0;
}
EXPORT_FUNC(cond_init);

int cond_wait(cond_t *cond) {
    kthread_t *thread = mtask_get_curr_thread();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if(!thread) { return -EUNSPEC; }
#endif

    cond_listitem_t *item = kmalloc(sizeof(cond_listitem_t));
    if(!item) { return -ENOMEM; }

    item->list_item.data = item;
    item->thread         = thread;

    lock(&cond->lock);
    llist_append(&cond->list, &item->list_item);
    unlock(&cond->lock);

    /* @todo Remove thread from scheduler */
    thread->cond = cond;
    run_sched();

    return 0;
}
EXPORT_FUNC(cond_wait);

static int _unblock_thread(cond_listitem_t *item) {
    ((kthread_t *)item->thread)->cond = NULL;
    kfree(item);

    return 0;
}

int cond_signal(cond_t *cond) {
    llist_item_t *item;

    lock(&cond->lock);
    while((item = llist_pop_unlocked(&cond->list))) {
        if(_unblock_thread(item->data)) {
            unlock(&cond->lock);
            return -EUNSPEC;
        }
    }
    unlock(&cond->lock);

    return 0;
}
EXPORT_FUNC(cond_signal);
