#ifndef ATOMIC_H
#define ATOMIC_H

#include <intr/int.h>
#include <types.h>

#ifdef __clang__
	typedef _Atomic(int) lock_t;
#else
	typedef int lock_t;
#endif

// Compatibility between clang and GCC
#if __has_builtin(__c11_atomic_store)
	#define a_store __c11_atomic_store
#else
	#define a_store __atomic_store_n
#endif

#if __has_builtin(__c11_atomic_compare_exchange_weak) && __has_builtin(__c11_atomic_compare_exchange_strong)
	#define a_cmp_chx_weak   __c11_atomic_compare_exchange_weak
	#define a_cmp_chx_strong __c11_atomic_compare_exchange_strong
#else
	#define a_cmp_chx_weak(ptr, exp, des, smm, emm)   __atomic_compare_exchange_n(ptr, exp, des, 1, smm, emm)
	#define a_cmp_chx_strong(ptr, exp, des, smm, emm) __atomic_compare_exchange_n(ptr, exp, des, 0, smm, emm)
#endif


static inline void unlock(lock_t *lock)
{
	a_store(lock, 0, __ATOMIC_RELEASE);
}

static inline void lock(lock_t *lock)
{
	int old = 0;
	while(!a_cmp_chx_weak(lock, &old, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
	{
		busy_wait();
		old = 0;
	}
}

#endif