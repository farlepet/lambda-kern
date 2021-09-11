#include <proc/atomic.h>
#include <proc/thread.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>

#include <string.h>

kthread_t *thread_create(uintptr_t entrypoint, void * data, const char *name, size_t stack_size, int prio) {
    kthread_t *thread = (kthread_t *)kmalloc(sizeof(kthread_t));
    if(thread == NULL) {
        return NULL;
    }
	
    if(!stack_size) stack_size = DEFAULT_STACK_SIZE;
    
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
        return -1;
    }
 
	kthread_t *thread = thread_create(entrypoint, data, name, stack_size, prio);
    if(thread == NULL) {
        return -1;
    }
 
    proc_add_thread(curr_proc, thread);
    
    arch_setup_thread(thread);

    kdebug(DEBUGSRC_PROC, "kthread_create [%s] @ %08X | TID: %d", name, entrypoint, thread->tid);
    
    thread->flags      = KTHREAD_FLAG_RUNNABLE | KTHREAD_FLAG_RANONCE;
	
    sched_enqueue_thread(thread);
    
    return thread->tid;
}
