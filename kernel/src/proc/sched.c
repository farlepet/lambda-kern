#include <proc/proc.h>
#include <proc/thread.h>
#include <data/llist.h>
#include <mm/alloc.h>
#include <err/error.h>
#include <err/panic.h>
#include <video.h>

static llist_t     _thread_queue;
static llist_t    *_cpu_threads = NULL;
static kthread_t **_curr_thread = NULL;
static unsigned    _n_cpus      = 0;

static inline void _cpu_add_thread(unsigned, kthread_t *);

/* TODO: Keep track of usage on each CPU, and schedule accordingly */

int sched_init(unsigned n_cpus) {
    _cpu_threads = (llist_t *)   kmalloc(n_cpus * sizeof(llist_t));
    _curr_thread = (kthread_t **)kmalloc(n_cpus * sizeof(kthread_t *));

    for(unsigned i = 0; i < n_cpus; i++) {
        llist_init(&_cpu_threads[i]);
        _curr_thread[i] = NULL;
    }

    llist_init(&_thread_queue);

    /* TODO: Add idle thread to each list */

    _n_cpus = n_cpus;

    return 0;
}

__noreturn
static void _idle_thread(void) {
    for(;;) {
        busy_wait();
    }
}

void sched_idle_init(void) {
    kdebug(DEBUGSRC_PROC, ERR_INFO, "sched_idle_init(): Setting up idle threads for %d CPUs", _n_cpus);
    
    kproc_t *proc = proc_create("idle", 1, NULL);
    if(proc == NULL) {
        kpanic("sched_idle_init: Could not create idle process!");
    }
    
    for(unsigned cpu = 0; cpu < _n_cpus; cpu++) {
        kdebug(DEBUGSRC_PROC, ERR_DEBUG, "  sched_idle_init(): CPU %d", cpu);
        char name[9];
        /* TODO: Implement snprintf for safety */
        sprintf(name, "idle_%03d", cpu);
        
        kthread_t *thread = thread_create((uintptr_t)_idle_thread, NULL, name, 0x1000, PRIO_IDLE);
        if(thread == NULL) {
            kpanic("sched_idle_init: Could not create idle thread for CPU %u!", cpu);
        }
        thread->sched_item.data = thread;

        arch_setup_process(proc);
        proc->type |= TYPE_RUNNABLE;

        proc_add_thread(proc, thread);
        
        arch_setup_thread(thread);
        thread->flags |= KTHREAD_FLAG_RUNNABLE;

        _cpu_add_thread(cpu, thread);
    }
}

int sched_enqueue_thread(kthread_t *thread) {
    if(thread == NULL) {
        kpanic("sched_enqueue_thread(): thread is NULL!");
    }
    thread->sched_item.data = thread;
    
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "sched_enqueue(): TID: %d | NAME: %s", thread->tid, thread->name);
    
    if(lock_for(&_thread_queue.lock, 2000)) {
        return -1;
    }
 
    llist_append_unlocked(&_thread_queue, &thread->sched_item);

    unlock(&_thread_queue.lock);
    
    return 0;
}


static inline size_t _cpu_thread_count(unsigned cpu) {
    return llist_count(&_cpu_threads[cpu]);
}

static inline void _cpu_add_thread(unsigned cpu, kthread_t *thread) {
	kdebug(DEBUGSRC_PROC, ERR_TRACE, "Assigning thread (%u, %s) to cpu %u", thread->tid, thread->name, cpu);

    llist_append(&_cpu_threads[cpu], &thread->sched_item);
    if(!_curr_thread[cpu]) {
        _curr_thread[cpu] = thread;
    }
}

static inline void _schedule_thread(kthread_t *thread) {
    if(thread == NULL) {
        kpanic("_schedule_thread(): thread is NULL!");
    }
    
    unsigned cpu   = 0;
    size_t   count = _cpu_thread_count(0);

    for(unsigned i = 1; i < _n_cpus; i++) {
        size_t cnt = _cpu_thread_count(_n_cpus);
        if(cnt < count) {
            count = cnt;
            cpu   = i;
        }        
    }

    _cpu_add_thread(cpu, thread);
}

__hot
void sched_processes() {
    /* TODO: Check for stagnant threads on CPUs */

    /* TODO: Read dead threads/processes */

    /* NOTE: With the current design, we cannot wait on the lock, as other threads
     * are not running (at least on single-CPU systems) */
    if(lock_try(&_thread_queue.lock)) {
        return;
    }

    /* NOTE: Cannot use llist_iterator_t as we are removing the item before
     * iterating to the next one. */
    llist_item_t *item = llist_pop_unlocked(&_thread_queue);
    while(item) {
        _schedule_thread((kthread_t *)item->data);
        
        item = llist_pop_unlocked(&_thread_queue);
    }

    unlock(&_thread_queue.lock);
}

__hot
kthread_t *sched_get_curr_thread(unsigned cpu) {
    return _curr_thread[cpu];
}

__hot
kthread_t *sched_next_process(unsigned cpu) {
    /*
     * Next thread is chosen by going process-by-process, looking at each thread
     * until we find one that we can schedule.
     */
    
    kthread_t *thread = _curr_thread[cpu];
    
    llist_iterator_t iter = {
        .first = (llist_item_t *)0xFFFFFFFF,
        .curr  = &thread->sched_item
    };

    do {
        if(!llist_iterate(&iter, (void **)&thread)) {
            kpanic("sched_next_process(): Could not schedule new task -- All tasks are blocked!");
        }

        if(iter.first == (llist_item_t *)0xFFFFFFFF) {
            iter.first = iter.curr;
        }

        if(!thread) {
            kpanic("sched_next_process(): Thread is NULL!");
        }
        if(!thread->process) {
            kpanic("sched_next_process(): Thread has no associated process!");
        }
    } while(!(thread->process->type & TYPE_RUNNABLE)         ||
            !(thread->flags         & KTHREAD_FLAG_RUNNABLE) ||
            thread->blocked);

    _curr_thread[cpu] = thread;

    _curr_thread[cpu]->process->book.schedule_count++;

    return _curr_thread[cpu];
}
