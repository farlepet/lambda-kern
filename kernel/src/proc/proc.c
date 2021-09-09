#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <string.h>


int proc_add_file(kproc_t *proc, kfile_hand_t *file) {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if(proc->open_files[i] == NULL) {
			proc->open_files[i] = file;
			return i;
		}
	}

	return -1;
}

int proc_add_mmap_ent(kproc_t *proc, uintptr_t virt_address, uintptr_t phys_address, size_t length) {
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

int proc_add_mmap_ents(kproc_t *proc, struct kproc_mem_map_ent *entries) {
	if(proc->mmap == NULL) {
		proc->mmap = entries;
	} else {
		struct kproc_mem_map_ent *last = proc->mmap;
		while(last->next != NULL) last = last->next;
		last->next = entries;
	}

	return 0;
}

int proc_add_child(kproc_t *parent, kproc_t *child) {
	for(int i = 0; i < MAX_CHILDREN; i++) {
		if(!parent->children[i]) {
			parent->children[i] = child;
			return 0;
		}
	}

	return 1;
}



