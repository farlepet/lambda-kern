#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <string.h>

/* @todo remove */
void kproc_to_uproc(struct kproc *kp, struct uproc *up)
{
	memcpy(up->name, kp->name, 64);

    up->pid      = kp->pid;
    up->uid      = kp->uid;
    up->gid      = kp->gid;
    up->type     = kp->type;
    //up->blocked  = kp->blocked;
    up->exitcode = kp->exitcode;
    //up->prio     = kp->prio;

	memcpy(up->children, kp->children, MAX_CHILDREN * sizeof(int));

#ifdef ARCH_X86
	//up->ip = kp->arch.eip;
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

int proc_add_child(struct kproc *parent, struct kproc *child) {
	for(int i = 0; i < MAX_CHILDREN; i++) {
		if(!parent->children[i]) {
			parent->children[i] = child;
			return 0;
		}
	}

	return 1;
}


__hot void sched_processes()
{
	// Nothing to do with  current state of the scheduler
}

__hot void sched_next_process()
{
	/*
	 * Next thread is chosen by going process-by-process, looking at each thread
	 * until we find one that we can schedule.
	 */
	int nr = 0;

	kproc_t *next   = curr_proc;
	uint32_t thread = curr_thread + 1;
	if(thread >= MAX_THREADS) {
		next   = (kproc_t *)curr_proc->list_item.next->data;
		thread = 0;
	}

	/* @todo The present implementation is somewhat ineffecient. */
	while(!(next->type & TYPE_RUNNABLE)                       ||
	      !(next->threads[thread].flags & KTHREAD_FLAG_VALID) ||
		  next->threads[thread].blocked) {

	    /*if(next->threads[thread].flags & KTHREAD_FLAG_VALID) {
            kdebug(DEBUGSRC_PROC, "TID: %d | BLK: %08X | PROC: %s", next->threads[thread].tid, next->threads[thread].blocked, next->name);
		}*/

		thread++;
		if (thread >= MAX_THREADS) {
			if((next->list_item.next       == NULL) ||
			   (next->list_item.next->data == NULL)) {
				kpanic("Next task is NULL!");
			}
			next = (kproc_t *)next->list_item.next->data;

			thread = 0;
			
			if(next == curr_proc) nr++;
			if(nr >= 2) {
				// We couldn't get a new runnable task, so we panic and halt
				kpanic("Could not schedule new task -- All tasks are blocked!");
			}
		}
	}
	curr_proc   = next;
	curr_thread = thread;

	curr_proc->book.schedule_count++;
}
