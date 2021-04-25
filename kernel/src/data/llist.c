#include <types.h>
#include <string.h>

#include <data/llist.h>

void llist_init(llist_t *list) {
    memset(list, 0, sizeof(llist_t));
}

void llist_append(llist_t *list, llist_item_t *item) {
    if((list == NULL) ||
       (item == NULL)) {
        return;
    }

    lock(&list->lock);

    if(list->list) {
        list->list->prev->next = item;
        item->prev             = list->list->prev;
        list->list->prev       = item;
        item->next             = list->list;
    } else {
        list->list = item;
        item->next = item;
        item->prev = item;
    }

    unlock(&list->lock);
}

void llist_remove(llist_t *list, llist_item_t *item) {
    if((list       == NULL) ||
       (item       == NULL) ||
       (list->list == NULL)) {
        return;
    }

    lock(&list->lock);

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

    unlock(&list->lock);
}
