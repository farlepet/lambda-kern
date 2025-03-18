#ifndef DATA_TYPES_LLIST_H
#define DATA_TYPES_LLIST_H


typedef struct llist_item     llist_item_t;
typedef struct llist          llist_t;
typedef struct llist_iterator llist_iterator_t;

#include <proc/atomic/types/lock.h>

/**
 * Represents an item in a linked list
 */
struct llist_item {
    void         *data; /** Data represented by list item */
    llist_item_t *prev; /** Previous item in list */
    llist_item_t *next; /** Next item in list */
};

/**
 * Represents a linked list
 */
struct llist {
    llist_item_t *list; /** Pointer to first item in list */
    lock_t        lock; /** Lock to ensure only a single thread touches the list at a time */
};

/**
 * llist iterator state
 */
struct llist_iterator {
    llist_item_t *first; /** First item encountered in list */
    llist_item_t *curr;  /** Current list item */
};

#endif

