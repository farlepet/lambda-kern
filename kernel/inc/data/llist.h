#ifndef DATA_LLIST_H
#define DATA_LLIST_H

#include <proc/atomic.h>

typedef struct llist_item llist_item_t;
typedef struct llist      llist_t;

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
 * \brief Initialize linked list
 * 
 * @param list List to initialize
 */
void llist_init(llist_t *list);

/**
 * \brief Append the given item to the list
 *
 * @note Items are intended to be added one at a time. The item's prev/next
 *       will be overwritten.
 *  
 * @param list List to add item to
 * @param item Item to add to list
 */
void llist_append(llist_t *list, llist_item_t *item);

/**
 * \brief Remove the given item from the list
 * 
 * @param list List to remove item from
 * @param item Item to remove from list
 */
void llist_remove(llist_t *list, llist_item_t *item);

#endif
