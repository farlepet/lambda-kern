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

	memcpy(up->children, kp->children, MAX_CHILDREN * sizeof(int));

#ifdef ARCH_X86
	up->ip = kp->eip;
#endif
}