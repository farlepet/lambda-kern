#ifndef PROC_TYPES_COND_H
#define PROC_TYPES_COND_H

typedef struct cond_listitem cond_listitem_t;
typedef struct cond          cond_t;

#include <proc/types/kthread.h>
#include <data/llist.h>
#include <proc/atomic.h>

struct cond_listitem {
    llist_item_t list_item;
    kthread_t   *thread;
};

struct cond {
    lock_t  lock; /** Lock */
    llist_t list; /** Threads waiting on condition. */
};

#endif
