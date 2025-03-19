#include <errno.h>
#include <string.h>

#include <proc/atomic/lock.h>
#include <proc/thread.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>

kthread_t *thread_create(uintptr_t entrypoint, void * data, const char *name, size_t stack_size, int prio) {
    kthread_t *thread = (kthread_t *)kmalloc(sizeof(kthread_t));
    if(thread == NULL) {
        return NULL;
    }
    
    if(!stack_size) {
        stack_size = CONFIG_PROC_USER_STACK_SIZE;
    } else if(stack_size > CONFIG_PROC_USER_STACK_SIZE_MAX) {
        kpanic("Stack size requested (%u) is too large (> %u)", stack_size,
               CONFIG_PROC_USER_STACK_SIZE_MAX);
    }
    
    memset(thread, 0, sizeof(kthread_t));

    strncpy(thread->name, name, KPROC_NAME_MAX);
    
    thread->entrypoint  = entrypoint;
    thread->prio        = prio;
    thread->stack_size  = stack_size;
    thread->thread_data = data;
    
    return thread;
}

int thread_spawn(uintptr_t entrypoint, void *data, const char *name, size_t stack_size, int prio) {
    kproc_t *curr_proc = mtask_get_curr_process();
    if(curr_proc == NULL) {
        return -EUNSPEC;
    }
 
    kthread_t *thread = thread_create(entrypoint, data, name, stack_size, prio);
    if(thread == NULL) {
        return -EUNSPEC;
    }
 
    proc_add_thread(curr_proc, thread);
    
    arch_setup_thread(thread);

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "kthread_create [%s] @ %08X | TID: %d", name, entrypoint, thread->tid);
    
    thread->flags      = KTHREAD_FLAG_RUNNABLE | KTHREAD_FLAG_RANONCE;
    
    sched_enqueue_thread(thread);
    
    return thread->tid;
}

int thread_destroy(kthread_t *thread) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "thread_destroy: %d, %s", thread->tid, thread->name);
    sched_remove_thread(thread);

    kfree(thread);

    return 0;
}

int thread_get_tid(void) {
    const kthread_t *thread = mtask_get_curr_thread();
    if(!thread) {
        return -EUNSPEC;
    }
    return thread->tid;
}

