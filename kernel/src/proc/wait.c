#include <lambda/export.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>

/**
 * Find the first dead child process of the given parent.
 * 
 * @param parent Parent process
 * @returns Index of dead child process, else -1
 */
static inline int find_dead_child(struct kproc *parent) {
    for(int idx = 0; idx < MAX_CHILDREN; idx++) {
        if(parent->children[idx]) {
            if(parent->children[idx]->type & TYPE_ZOMBIE) { // Is this process dead?
                return idx;
            }
        }
    }

    return -1;
}

int wait(int *stat_loc) {
    int chidx        = 0;
    int cpid         = 0;
    int child_exists = 0;
    struct kproc *child = NULL;

    // Check that the process has a child:
    for(int ch = 0; ch < MAX_CHILDREN; ch++) {
        if(curr_proc->children[ch]) {
            child_exists = 1;
            break;
        }
    }

    // If process has no children, return -1
    if(!child_exists) {
        return -1;
    }

    // Check if a dead child process already exists:
    chidx = find_dead_child(curr_proc);
    if(chidx >= 0) {
        goto CHILD_FOUND;
    }

    // Block and wait for scheduler
    curr_thread->blocked |= BLOCK_WAIT;
    //interrupt_halt();
    run_sched();

    // When process is re-entered here, a child has exited
    chidx = find_dead_child(curr_proc);
    if(chidx < 0) {
        // Something went wrong, we should never get here
        return -1;
    }
CHILD_FOUND:
    child = curr_proc->children[chidx];

    (void)stat_loc; // TODO: Modify stat_loc

    // TODO: Free up process-used memory.
    // Process slot can now be safely re-used
    if(child->type & TYPE_ZOMBIE) {
        child->type = 0;
    }

    // Free up child slot
    curr_proc->children[chidx] = NULL;

    return cpid;
}
EXPORT_FUNC(wait);
