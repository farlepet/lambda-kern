#include "errno.h"
#include <arch/intr/int.h>
#include <arch/proc/tasking.h>

#include <proc/atomic/lock.h>
#include <proc/mtask.h>
#include <proc/cond.h>
#include <proc/thread.h>
#include <proc/proc.h>
#include <err/error.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>
#include <mm/mm.h>
#include <fs/fs.h>

static llist_t procs;

static int next_pid = 1;

lock_t creat_task = 0; //!< Lock used when creating tasks

struct kproc *proc_by_pid(int pid) {
    if(!procs.list) {
        return NULL;
    }

    kproc_t         *proc;
    llist_iterator_t p_iter;
    llist_iterator_init(&procs, &p_iter);
    while(llist_iterate(&p_iter, (void **)&proc)) {
        if(proc->pid == pid) {
            return proc;
        }
    }
    
    return NULL;
}

kthread_t *thread_by_tid(int tid) {
    if(!procs.list) {
        return NULL;
    }

    llist_iterator_t p_iter;
    llist_iterator_t t_iter;

    kproc_t   *proc;
    kthread_t *thread;

    llist_iterator_init(&procs, &p_iter);
    while(llist_iterate(&p_iter, (void **)&proc)) {
        llist_iterator_init(&proc->threads, &t_iter);
        while(llist_iterate(&t_iter, (void **)&thread)) {
            if(thread->tid == (uint32_t)tid) {
                return thread;
            }
        }
    }
    
    return NULL;
}

int get_pid() {
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) {
        return -EUNSPEC;
    } else {
        return thread->process->pid;
    }
}

int get_next_pid() {
    return next_pid++;
}

int proc_create_stack(kthread_t *thread) {
    return arch_proc_create_stack(thread);
}

int proc_create_kernel_stack(kthread_t *thread) {
    return arch_proc_create_kernel_stack(thread);
}


int add_task(void *process, char* name, uint32_t stack_size, int pri, int domain, mmu_table_t *mmu_table) {
    lock(&creat_task);

    kproc_t *curr_proc = mtask_get_curr_process();
    
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "mtask:add_task(%p, %s, ...)", process, name);

    /*
     * Create process
     */

    kproc_t *proc = proc_create(name, domain, mmu_table);
    if(!proc) {
        kdebug(DEBUGSRC_PROC, ERR_CRIT, "mtask:add_task: Could not create process.");
        return -EUNSPEC;
    }

    if(curr_proc && proc_add_child(curr_proc, proc)) {
        kdebug(DEBUGSRC_PROC, ERR_DEBUG, "mtask:add_task: Process %d has run out of children slots", curr_proc->pid);
        unlock(&creat_task);
        kfree(proc);
        return 0;
    }
    
    // TODO:
    proc->uid  = 0;
    proc->gid  = 0;
    proc->type = TYPE_RUNNABLE;

    arch_setup_process(proc);

    /*
     * Create thread
     */

    kthread_t *thread = thread_create((uintptr_t)process, NULL, name, stack_size, pri);

    proc_add_thread(proc, thread);

    arch_setup_thread(thread);

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "PID: %d MMU: %08X", proc->pid, proc->mmu_table);

    thread->flags |= KTHREAD_FLAG_RUNNABLE | KTHREAD_FLAG_RANONCE;
    
    mtask_insert_proc(proc);
    sched_enqueue_thread(thread);

    unlock(&creat_task);

    return proc->pid;
}


int mtask_insert_proc(struct kproc *proc) {
    proc->list_item.data = proc;
    llist_append(&procs, &proc->list_item);

    return 0;
}

int mtask_remove_proc(kproc_t *proc) {
    llist_remove(&procs, &proc->list_item);

    return 0;
}

__hot
kthread_t *mtask_get_curr_thread(void) {
    /* TODO: Determine current CPU */
    return sched_get_curr_thread(0);
}

__hot
kproc_t *mtask_get_curr_process(void) {
    /* TODO: Determine current CPU */
    kthread_t *thread = sched_get_curr_thread(0);
    if(thread == NULL) {
        return NULL;
    }
    return thread->process;
}


void init_multitasking(void *process, char *name) {
    kdebug(DEBUGSRC_PROC, ERR_INFO, "Initializing multitasking");

    llist_init(&procs);
    sched_idle_init();

    int tid = add_task(process, name, 0x2000, PRIO_KERNEL, PROC_DOMAIN_KERNEL, NULL);
    kthread_t *thread = thread_by_tid(tid);
    if(thread == NULL) {
        kpanic("Could not find initial kernel thread!");
    }

    arch_multitasking_init();

    kdebug(DEBUGSRC_PROC, ERR_INFO, "Multitasking enabled");
}



__noreturn void exit(int code) {
    kproc_t *curr_proc = mtask_get_curr_process();

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "exit(%d) called by process %d.", code, curr_proc->pid);

    /* @todo Migrate this to threads*/
    curr_proc->type &= (uint32_t)~(TYPE_RUNNABLE);
    curr_proc->type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
    curr_proc->exitcode = code;

    // If parent process is waiting for child to exit, allow it to continue execution:
    if(curr_proc->parent) {
        kproc_t *parent = proc_by_pid(curr_proc->parent);
        if(parent->wait_cond) {
            cond_signal(parent->wait_cond);
            kfree(parent->wait_cond);
            parent->wait_cond = NULL;
        }
    }

    for(;;) {
        run_sched();
    }
}
