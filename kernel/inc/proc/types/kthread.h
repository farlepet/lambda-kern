#ifndef PROC_TYPES_KTHREAD_H
#define PROC_TYPES_KTHREAD_H

typedef struct kthread       kthread_t;
typedef struct kthread_stats kthread_stats_t;

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
#include <proc/types/cond.h>
#include <arch/proc/tasking.h>
#include <data/llist.h>

struct kthread_stats {
    uint64_t sched_time_last;  /** Time at which thread was last scheduled */
    uint64_t sched_time_accum; /** Accumulated time for which this thread has been scheduled, in nanoseconds */
    uint32_t sched_count;      /** Number of times this thread has been scheduled */
};

struct kthread {
    char              name[KPROC_NAME_MAX+1];   /** Name of thread */
    uint32_t          tid;        /** Thread ID */
    uint32_t          flags;      /** Thread flags */
    int               prio;       /** Thread priority */

    cond_t           *cond;       /** Conditional thread is waiting on, if applicable */

    struct kproc     *process;    /** Pointer to owning process */

    kthread_arch_t    arch;        /** Architecture-specific thread data */
    uintptr_t         entrypoint;  /** Program start */
    size_t            stack_size;  /** Size of stack */
    void             *thread_data; /** Data to be provided to thread as an argument */

    kthread_stats_t   stats;       /** Thread statistics */

    llist_item_t      list_item;   /** List item for owning process */
    llist_item_t      sched_item;  /** List item for scheduling */
};

#endif
