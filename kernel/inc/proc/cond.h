#ifndef PROC_COND_H
#define PROC_COND_H

#include <mm/alloc.h>
#include <proc/types/cond.h>

/**
 * @brief Initialize condition variable
 *
 * @param cond Condition veriable
 * @return int 0 on success, else non-zero
 */
int cond_init(cond_t *cond);

/**
 * @brief Allocate and initialize a condition variable
 *
 * @return Pointer to condition variable on success, else NULL
 */
static inline cond_t *cond_create(void) {
    cond_t *cond = kmalloc(sizeof(cond_t));
    if(!cond) {
        return NULL;
    }
    /* Currently, cond_init never returns error */
    cond_init(cond);
    return cond;
}

/**
 * @brief Wait for condition variable to be signaled
 *
 * @param cond Condition variable
 * @return int 0 on success, else non-zero
 */
int cond_wait(cond_t *cond);

/**
 * @brief Wakeup threads waiting on condition varialbe
 *
 * @param cond Condition variable
 * @return int 0 on success, else non-zero
 */
int cond_signal(cond_t *cond);

#endif
