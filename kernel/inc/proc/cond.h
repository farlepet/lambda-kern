#ifndef PROC_COND_H
#define PROC_COND_H

#include <proc/types/cond.h>

/**
 * @brief Initialize condition variable
 *
 * @param cond Condition veriable
 * @return int 0 on success, else non-zero
 */
int cond_init(cond_t *cond);

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
