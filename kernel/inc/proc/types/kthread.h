#ifndef PROC_TYPES_KTHREAD_H
#define PROC_TYPES_KTHREAD_H

typedef struct kthread kthread_t;

#define PRIO_IDLE       0 //!< Only idle processes use this priority
#define PRIO_USERPROG   1 //!< Priority for user programs
#define PRIO_KERNELPROG 2 //!< Priority for kernel programs
#define PRIO_DRIVER     3 //!< Priority for kernel drivers
#define PRIO_KERNEL     4 //!< Priority for main kernel tasks

#define KTHREAD_FLAG_RUNNABLE   (1UL << 31) /** Thread contents is valid */
#define KTHREAD_FLAG_RUNNING    (1UL <<  0) /** Thread is currently running */
#define KTHREAD_FLAG_RANONCE    (1UL <<  1) /** Thread has ran at least once */
#define KTHREAD_FLAG_STACKSETUP (1UL <<  2) /** User stack has been initialized already */

#include <proc/types/kproc.h>
#include <arch/proc/tasking.h>
#include <data/llist.h>

struct kthread {
	char              name[KPROC_NAME_MAX+1];   /** Name of thread */
	uint32_t          tid;        /** Thread ID */
	uint32_t          flags;      /** Thread flags */
	int               prio;       /** Thread priority */

	volatile uint32_t blocked;    /** Contains flags telling whether or not this thread is blocked, and by what */

	struct kproc     *process;    /** Pointer to owning process */

	kthread_arch_t    arch;        /** Architecture-specific thread data */
	uintptr_t         entrypoint;  /** Program start */
	size_t            stack_size;  /** Size of stack */
	void             *thread_data; /** Data to be provided to thread as an argument */
	
	llist_item_t      list_item;
	llist_item_t      sched_item;
};

#endif