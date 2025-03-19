#ifndef DATA_LLIST_H
#define DATA_LLIST_H

#include <stddef.h>

#include <data/types/llist.h>

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
 * \brief Append the given item to the list, without locking the list
 * 
 * @param list List to add item to
 * @param item Item to add to list
 */
void llist_append_unlocked(llist_t *list, llist_item_t *item);

/**
 * \brief Remove the given item from the list
 * 
 * @param list List to remove item from
 * @param item Item to remove from list
 */
void llist_remove(llist_t *list, llist_item_t *item);

/**
 * \brief Remove the given item from the list, without locking the list
 * 
 * @param list List to remove item from
 * @param item Item to remove from list
 */
void llist_remove_unlocked(llist_t *list, llist_item_t *item);

/**
 * \brief Get position of specified item in the list
 * 
 * @param list List item is a part of
 * @param item Item to get position of
 * 
 * @return -EINVAL if item not found, else 0-indexed position of item
 */
int llist_get_position(llist_t *list, llist_item_t *item);

/**
 * \brief Initialize llist iterator
 * 
 * @param list List to iterate over
 * @param iter Iterator to initialize
 */
void llist_iterator_init(llist_t *list, llist_iterator_t *iter);

/**
 * \brief Iterate over llist
 * 
 * @param iter Iterator object
 * @param data Value of current list item
 * 
 * @return 0 on invalid data or end of list, else 1
 */
int llist_iterate(llist_iterator_t *iter, void **data);

/**
 * \brief Remove an item off the end of the list and return it
 * 
 * @param list List to pop item off of
 * @return Pointer to list item, NULL on error
 */
llist_item_t *llist_pop_unlocked(llist_t *list);

/**
 * \brief Count number of items in a list
 * 
 * @param list List to count items of
 * @return -E* on error, else number of items in the list
 */
int llist_count(const llist_t *list);

/**
 * @brief Gets the list item at the specified index
 *
 * @param list List to index into
 * @param idx Index of list item to find, starting from zero
 * @return llist_item_t* Pointer to list item at requested index, or NULL if not found
 */
llist_item_t *llist_get(const llist_t *list, size_t idx);

#endif
