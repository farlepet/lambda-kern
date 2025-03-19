#include <errno.h>

#include <lambda/export.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <proc/cond.h>
#include <proc/exec.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>

/**
 * Find the first dead child process of the given parent.
 * 
 * @param parent Parent process
 * @returns Index of dead child process, else -1
 */
static inline int _find_dead_child(struct kproc *parent) {
    for(int idx = 0; idx < MAX_CHILDREN; idx++) {
        if(parent->children[idx]) {
            if(parent->children[idx]->type & TYPE_ZOMBIE) { // Is this process dead?
                return idx;
            }
        }
    }

    return -EUNSPEC;
}

int wait(int *stat_loc) {
    int      chidx        = 0;
    int      cpid         = 0;
    int      child_exists = 0;
    kproc_t *child        = NULL;

    kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = curr_thread->process;

    // Check that the process has a child:
    for(int ch = 0; ch < MAX_CHILDREN; ch++) {
        if(curr_proc->children[ch]) {
            child_exists = 1;
            break;
        }
    }

    // If process has no children, return -1
    if(!child_exists) {
        kdebug(DEBUGSRC_PROC, ERR_TRACE, "wait(): No children");
        return -EUNSPEC;
    }

    disable_interrupts();
    // Check if a dead child process already exists:
    chidx = _find_dead_child(curr_proc);
    if(chidx >= 0) {
        goto CHILD_FOUND;
    }

    if(!curr_proc->wait_cond) {
        curr_proc->wait_cond = kmalloc(sizeof(cond_t));
        cond_init(curr_proc->wait_cond);
    }

    // Block and wait for scheduler
    cond_wait(curr_proc->wait_cond);

    // When process is re-entered here, a child has exited
    chidx = _find_dead_child(curr_proc);
    if(chidx < 0) {
        // Something went wrong, we should never get here
        kpanic("wait(): Child missing");
    }
CHILD_FOUND:
    enable_interrupts();

    child = curr_proc->children[chidx];

    (void)stat_loc; // TODO: Modify stat_loc

    // Free up child slot
    curr_proc->children[chidx] = NULL;

    proc_destroy(child);

    return cpid;
}
EXPORT_FUNC(wait);
