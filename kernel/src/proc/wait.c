#include <proc/mtask.h>
#include <intr/int.h>

int wait(int *stat_loc) {
    int idx   = proc_by_pid(current_pid);
    int chidx = 0;
    int cpid  = 0;

    // Check that a child has not already exited
    for(int ch = 0; ch < MAX_CHILDREN; ch++) {
        if(procs[idx].children[ch]) {
            chidx = proc_by_pid(procs[idx].children[ch]);
            if(chidx >= 0) {
                if(procs[chidx].type & TYPE_VALID &&  // Is this a valid process entry?
                   procs[chidx].type & TYPE_ZOMBIE) { // Is this a dead process?
                    // We found a dead child, skip to the end
                    goto CHILD_FOUND;
                }
            }
        }
    }

    // Block and wait for scheduler
    procs[idx].blocked |= BLOCK_WAIT;
    interrupt_halt();

    // When process is re-entered here, child has exited
CHILD_FOUND:
    cpid = procs[chidx].pid;

    (void)stat_loc; // TODO: Modify stat_loc

    procs[chidx].type = TYPE_ZOMBIE | TYPE_REAP; // We don't need this process anymore

    return cpid;
}