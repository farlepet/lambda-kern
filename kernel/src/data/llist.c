#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <lambda/config_defs.h>
#include <lambda/export.h>

#include <data/llist.h>
#include <err/panic.h>
#include <mm/mm.h>
#include <proc/atomic/lock.h>

void llist_init(llist_t *list) {
    memset(list, 0, sizeof(llist_t));
}
EXPORT_FUNC(llist_init);

void llist_append_unlocked(llist_t *list, llist_item_t *item) {
    if((list == NULL) ||
       (item == NULL)) {
        return;
    }

#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    kassert(mm_check_addr(list), "Bad list addr: %p", list);
    kassert(mm_check_addr(item), "Bad item addr: %p", item);
#endif

    if(list->list) {
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
        kassert(mm_check_addr(list->list), "Bad list->list addr: %p", list->list);
        kassert(mm_check_addr(list->list->prev), "Bad list->list->prev addr: %p", list->list);
        kassert(mm_check_addr(list->list->prev->next), "Bad list->list->prev->next addr: %p", list->list);
#endif
        list->list->prev->next = item;
        item->prev             = list->list->prev;
        list->list->prev       = item;
        item->next             = list->list;
    } else {
        list->list = item;
        item->next = item;
        item->prev = item;
    }
}

void llist_append(llist_t *list, llist_item_t *item) {
    if((list == NULL) ||
       (item == NULL)) {
        return;
    }

    lock(&list->lock);

    llist_append_unlocked(list, item);

    unlock(&list->lock);
}
EXPORT_FUNC(llist_append);

void llist_remove_unlocked(llist_t *list, llist_item_t *item) {
    if((list       == NULL) ||
       (item       == NULL) ||
       (list->list == NULL)) {
        return;
    }

    if((list->list == item) &&
       (item->next == item->prev)) {
        list->list = NULL;
    } else {
        /* NOTE: This is assuming that item is actually a member of the list */
        item->prev->next = item->next;
        item->next->prev = item->prev;
    }

    /* Remove item's references to the list */
    item->next = NULL;
    item->prev = NULL;
}

void llist_remove(llist_t *list, llist_item_t *item) {
    if((list       == NULL) ||
       (item       == NULL) ||
       (list->list == NULL)) {
        return;
    }

    lock(&list->lock);

    llist_remove_unlocked(list, item);

    unlock(&list->lock);
}
EXPORT_FUNC(llist_remove);

int llist_get_position(llist_t *list, llist_item_t *item) {
    if((list == NULL) || (item == NULL)) {
        return -EINVAL;
    }

#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
    kassert(mm_check_addr(list->list), "Bad list->list addr: %p", list->list);
#endif

    llist_item_t *citem = list->list;
    int i = 0;
    do {
        if(citem == item) {
            return i;
        }
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_HIGHIMPACT)
        kassert(mm_check_addr(citem->next), "Bad citem->next addr: %p", citem->next);
#endif
        citem = citem->next;
        i++;
    } while(citem != list->list);

    return -EINVAL;
}
EXPORT_FUNC(llist_get_position);

void llist_iterator_init(llist_t *list, llist_iterator_t *iter) {
    if((list == NULL) ||
       (iter == NULL)) {
        return;
    }

    iter->first = list->list;
    iter->curr  = NULL;
}
EXPORT_FUNC(llist_iterator_init);

int llist_iterate(llist_iterator_t *iter, void **data) {
    if((iter        == NULL) ||
       (data        == NULL) ||
       (iter->first == NULL)) {
        return 0;
    }
    
    if(iter->curr == NULL) {
        iter->curr = iter->first;
    } else {
        iter->curr = iter->curr->next;
        if(iter->curr == iter->first) {
            /* Done iterating */
            return 0;
        }
    }

    if(iter->curr == NULL) {
        kpanic("llist_iterate: iter->curr = NULL");
    }

    *data = iter->curr->data;

    return 1;
}
EXPORT_FUNC(llist_iterate);

llist_item_t *llist_pop_unlocked(llist_t *list) {
    if((list       == NULL) ||
       (list->list == NULL)) {
        return NULL;
    }

    llist_item_t *item = list->list->prev;

    if(list->list == item) {
        list->list = NULL;
    } else {
        list->list->prev = item->prev;
        item->prev->next = list->list;
    }

    return item;
}

int llist_count(const llist_t *list) {
    if(list == NULL) {
        return -EINVAL;
    }
    if(list->list == NULL) {
        return 0;
    }

    int count = 0;
    const llist_item_t *item  = list->list;
    const llist_item_t *first = item;

    do {
        item = item->next;
        count++;
    } while(item != first);

    return count;
}
EXPORT_FUNC(llist_count);

llist_item_t *llist_get(const llist_t *list, size_t idx) {
    if(list == NULL || list->list == NULL) {
        return NULL;
    }

    size_t              count = 0;
    llist_item_t       *item  = list->list;
    const llist_item_t *first = item;

    do {
        if(count == idx) {
            return item;
        }
        item = item->next;
        count++;
    } while(item != first);

    return NULL;
}
EXPORT_FUNC(llist_get);
