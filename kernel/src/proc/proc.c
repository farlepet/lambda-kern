#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
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


int proc_add_file(struct kproc *proc, struct kfile *file) {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if(proc->open_files[i] == NULL) {
			proc->open_files[i] = file;
			return i;
		}
	}

	return -1;
}

int proc_add_mmap_ent(struct kproc *proc, uintptr_t virt_address, uintptr_t phys_address, size_t length) {
	struct kproc_mem_map_ent *ent = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));

	ent->virt_address = virt_address;
	ent->phys_address = phys_address;
	ent->length       = length;
	ent->next         = NULL;

	if(proc->mmap == NULL) {
		proc->mmap = ent;
	} else {
		struct kproc_mem_map_ent *last = proc->mmap;
		while(last->next != NULL) last = last->next;
		last->next = ent;
	}

	return 0;
}

int proc_add_mmap_ents(struct kproc *proc, struct kproc_mem_map_ent *entries) {
	if(proc->mmap == NULL) {
		proc->mmap = entries;
	} else {
		struct kproc_mem_map_ent *last = proc->mmap;
		while(last->next != NULL) last = last->next;
		last->next = entries;
	}

	return 0;
}

int proc_add_child(struct kproc *parent, int child_pid) {
	for(int i = 0; i < MAX_CHILDREN; i++) {
		if(!parent->children[i]) {
			parent->children[i] = child_pid;
			return 0;
		}
	}

	return 1;
}


__hot void sched_processes()
{
	// Nothing to do with  current state of the scheduler
}

__hot int sched_next_process()
{
	static int c_proc;
	int oc_proc = c_proc;
	int nr = 0;

	c_proc++;
	while(!(procs[c_proc].type & TYPE_RUNNABLE) || procs[c_proc].blocked)
	{
		if(c_proc == oc_proc) nr++;
		if(nr >= 2)
		{
			// We couldn't get a new runnable task, so we panic and halt
			kpanic("Could not schedule new task -- All tasks are blocked!");
		}

		c_proc++;
		if(c_proc >= MAX_PROCESSES) c_proc = 0;
	}
	current_pid = procs[c_proc].pid;

	procs[c_proc].book.schedule_count++;

	return c_proc;
}
