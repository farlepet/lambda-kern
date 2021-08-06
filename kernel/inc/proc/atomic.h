#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef __clang__
typedef _Atomic(int) lock_t;
#else
typedef int lock_t;
#endif

#include <arch/intr/int.h>

#include <time/time.h>
#include <types.h>

// Compatibility between clang and GCC
#if __has_builtin(__c11_atomic_store)
#  define a_store __c11_atomic_store
#else
#  define a_store __atomic_store_n
#endif

#if __has_builtin(__c11_atomic_compare_exchange_weak) && __has_builtin(__c11_atomic_compare_exchange_strong)
#  define a_cmp_chx_weak   __c11_atomic_compare_exchange_weak
#  define a_cmp_chx_strong __c11_atomic_compare_exchange_strong
#else
#  define a_cmp_chx_weak(ptr, exp, des, smm, emm)   __atomic_compare_exchange_n(ptr, exp, des, 1, smm, emm)
#  define a_cmp_chx_strong(ptr, exp, des, smm, emm) __atomic_compare_exchange_n(ptr, exp, des, 0, smm, emm)
#endif

/**
 * @brief Release an atomic lock
 * 
 * @param lock Lock to release
 */
static inline void unlock(lock_t *lock) {
	a_store(lock, 0, __ATOMIC_RELEASE);
}

/**
 * @brief Aquire an atomic lock
 * 
 * @param lock Lock to aquire
 */
void lock(lock_t *lock);

/**
 * @brief Aquire atomic lock, with timeout
 * 
 * @param lock Lock to attempt to aquire
 * @param ticks How long to attempt to aquire lock, in milliseconds
 * @return int 0 if successful, else 1
 */
int lock_for(lock_t *lock, uint32_t ms);

#endif
