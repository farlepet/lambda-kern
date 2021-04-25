#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>

#include <string.h>

int kthread_create(void *entrypoint, void *data, const char *name, size_t stack_size, int prio) {
	kthread_t *thread = (kthread_t *)kmalloc(sizeof(kthread_t));
    if(thread == NULL) {
        kpanic("kthread_create: Ran out of memory attempting to allocate thread!");
    }
    memset(thread, 0, sizeof(kthread_t));
	thread->list_item.data = thread;
	llist_append(&curr_proc->threads, &thread->list_item);


    if(name) {
        strncpy(thread->name, name, sizeof(thread->name) - 1);
        thread->name[sizeof(thread->name) - 1] = '\0';
    } else {
        strcpy(thread->name, curr_proc->name);
    }

    if(&thread->list_item != curr_proc->threads.list) {
        thread->tid    = get_next_pid();
    } else {
        /* @todo What if the first thread exited while other(s) continued? */
        thread->tid    = curr_proc->pid;
    }
    thread->entrypoint = (ptr_t)entrypoint;
    thread->process    = curr_proc;
    thread->prio       = prio;
	
    kdebug(DEBUGSRC_PROC, "kthread_create [%s] @ %08X | TID: %d", name, entrypoint, thread->tid);
    
    arch_setup_thread(thread, entrypoint, stack_size, data);

    thread->flags      = KTHREAD_FLAG_VALID | KTHREAD_FLAG_RANONCE;
    
    return thread->tid;
}
