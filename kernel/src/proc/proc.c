#include <proc/mtask.h>
#include <proc/proc.h>
#include <string.h>

void kproc_to_uproc(struct kproc *kp, struct uproc *up)
{
	memcpy(up->name, kp->name, 64);

    up->pid      = kp->pid;
    up->uid      = kp->uid;
    up->gid      = kp->gid;
    up->type     = kp->type;
    up->blocked  = kp->blocked;
    up->exitcode = kp->exitcode;
    up->prio     = kp->prio;

	memcpy(up->children, kp->children, MAX_CHILDREN * sizeof(int));

#ifdef ARCH_X86
	up->ip = kp->eip;
#endif
}




__hot void sched_processes()
{
	// Nothing to do with  current state of the scheduler
}

__hot int sched_next_process()
{
	static int c_proc;

	c_proc++;
	while(!(procs[c_proc].type & TYPE_RUNNABLE) || procs[c_proc].blocked)
		{ c_proc++; if(c_proc >= MAX_PROCESSES) c_proc = 0; }
	current_pid = procs[c_proc].pid;

	return c_proc;
}