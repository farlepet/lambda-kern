#include <lambda/export.h>

#include <stdint.h>
#include <string.h>

#include <data/llist.h>

void llist_init(llist_t *list) {
    memset(list, 0, sizeof(llist_t));
}
EXPORT_FUNC(llist_init);

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
EXPORT_FUNC(llist_append);

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
EXPORT_FUNC(llist_remove);

int llist_get_position(llist_t *list, llist_item_t *item) {
    if((list == NULL) ||
       (item == NULL)) {
        return -1;
    }

    llist_item_t *citem = list->list;
    int i = 0;
    do {
        if(citem == item) {
            return i;
        }
        citem = citem->next;
        i++;
    } while(citem != list->list);

    return -1;
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

    *data = iter->curr->data;

    return 1;
}
EXPORT_FUNC(llist_iterate);
