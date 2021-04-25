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

__optimize_none
__hot void sched_next_process()
{
	/*
	 * Next thread is chosen by going process-by-process, looking at each thread
	 * until we find one that we can schedule.
	 */

	kproc_t         *proc   = curr_proc;
	kthread_t       *thread = curr_thread;
	llist_iterator_t p_iter = {
		.first = (llist_item_t *)0xFFFFFFFF,
		.curr  = &proc->list_item
	};
	llist_iterator_t t_iter = {
		.first = proc->threads.list,
		.curr  = &curr_thread->list_item
	};

	/* @todo The present implementation is somewhat ineffecient. */
	do {

	    /*if(next->threads[thread].flags & KTHREAD_FLAG_VALID) {
            kdebug(DEBUGSRC_PROC, "TID: %d | BLK: %08X | PROC: %s", next->threads[thread].tid, next->threads[thread].blocked, next->name);
		}*/

		if(!llist_iterate(&t_iter, (void **)&thread)) {
			if((proc->list_item.next       == NULL) ||
			   (proc->list_item.next->data == NULL)) {
				kpanic("Next task is NULL!");
			}
			if(!llist_iterate(&p_iter, (void **)&proc)) {
				kpanic("Could not schedule new task -- All tasks are blocked!");
			}
			if(p_iter.curr == (llist_item_t *)0xFFFFFFFF) {
				/* We don't want to fault unless we go through all of the
				 * process a second time. */
				p_iter.first = p_iter.curr;
			}

			llist_iterator_init(&proc->threads, &t_iter);
			if(!llist_iterate(&t_iter, (void **)&thread)) {
				kpanic("Process contains no threads!");
			}
		}
	} while(!(proc->type & TYPE_RUNNABLE)         ||
	        !(thread->flags & KTHREAD_FLAG_VALID) ||
			thread->blocked);

	curr_proc   = proc;
	curr_thread = thread;

	curr_proc->book.schedule_count++;
}
