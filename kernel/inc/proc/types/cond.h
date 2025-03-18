#ifndef PROC_TYPES_COND_H
#define PROC_TYPES_COND_H

typedef struct cond_listitem cond_listitem_t;
typedef struct cond          cond_t;

#include <data/types/llist.h>
#include <proc/atomic/types/lock.h>

struct cond {
    lock_t  lock; /** Lock */
    llist_t list; /** Threads waiting on condition. */
};

struct cond_listitem {
    llist_item_t list_item;
    /** Pointer to thread that is waiting on the cond */
    void        *thread;
};


#endif
