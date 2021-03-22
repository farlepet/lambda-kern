#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>

#include <string.h>

/* @todo Linked-list of kthreads, rather than a static array */
static int _kthread_find_empty_slot() {
    for(int i = 0; i < MAX_THREADS; i++) {
        if(!(curr_proc->threads[i].flags & KTHREAD_FLAG_VALID)) {
            return i;
        }
    }

    return -1;
}

int kthread_create(void *entrypoint, void *data, const char *name, size_t stack_size, int prio) {
    int slot = _kthread_find_empty_slot();
    if(slot < 0) {
        kerror(ERR_MEDERR, "kthread_create: No empty thread slots for %d [%s]", curr_proc->pid, curr_proc->name);
        return -1;
    }

    kthread_t *thread = &curr_proc->threads[slot];

    memset(thread, 0, sizeof(kthread_t));

    if(name) {
        strncpy(thread->name, name, sizeof(thread->name) - 1);
        thread->name[sizeof(thread->name) - 1] = '\0';
    } else {
        strcpy(thread->name, curr_proc->name);
    }

    if(slot) {
        thread->tid    = get_next_pid();
    } else {
        /* @todo What if the first thread exited while other(s) continued? */
        thread->tid    = curr_proc->pid;
    }
    thread->entrypoint = (ptr_t)entrypoint;
    thread->process    = curr_proc;
    thread->prio       = prio;
	
    /* Set up message buffer */
	thread->messages.head  = 0;
	thread->messages.tail  = 0;
	thread->messages.count = 0;
	thread->messages.size  = MSG_BUFF_SIZE;
	thread->messages.buff  = thread->msg_buff;


    kdebug(DEBUGSRC_PROC, "kthread_create [%s] @ %08X | IDX: %d TID: %d", name, entrypoint, slot, thread->tid);
    
    arch_setup_thread(thread, entrypoint, stack_size, data);

    thread->flags      = KTHREAD_FLAG_VALID | KTHREAD_FLAG_RANONCE;
    
    return thread->tid;
}
