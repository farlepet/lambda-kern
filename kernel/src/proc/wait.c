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
            int chidx = proc_by_pid(parent->children[idx]);
            if(chidx >= 0) {
                if(procs[chidx].type & TYPE_VALID &&  // Is this a valid process entry?
                   procs[chidx].type & TYPE_ZOMBIE) { // Is this process dead?
                    return chidx;
                }
            }
        }
    }

    return -1;
}

int wait(int *stat_loc) {
    int idx          = proc_by_pid(current_pid);
    int chidx        = 0;
    int cpid         = 0;
    int child_exists = 0;

    // Check that the process has a child:
    for(int ch = 0; ch < MAX_CHILDREN; ch++) {
        if(procs[idx].children[ch]) {
            child_exists = 1;
            break;
        }
    }

    // If process has no children, return -1
    if(!child_exists) {
        return -1;
    }

    // Check if a dead child process already exists:
    chidx = find_dead_child(&procs[idx]);
    if(chidx >= 0) {
        goto CHILD_FOUND;
    }

    // Block and wait for scheduler
    procs[idx].blocked |= BLOCK_WAIT;
    //interrupt_halt();
    run_sched();

    // When process is re-entered here, a child has exited
    chidx = find_dead_child(&procs[idx]);
    if(chidx < 0) {
        // Something went wrong, we should never get here
        return -1;
    }
CHILD_FOUND:
    cpid = procs[chidx].pid;

    (void)stat_loc; // TODO: Modify stat_loc

    // TODO: Free up process-used memory.
    // Process slot can now be safely re-used
    if(procs[chidx].type & TYPE_ZOMBIE) {
        procs[chidx].type = 0;
    }

    return cpid;
}