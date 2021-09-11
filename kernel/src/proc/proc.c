#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <fs/fs.h>

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
	child->parent = parent->pid;
	for(int i = 0; i < MAX_CHILDREN; i++) {
		if(!parent->children[i]) {
			parent->children[i] = child;
			return 0;
		}
	}

	return 1;
}

kproc_t *proc_create(char *name, int kernel, arch_task_params_t *arch_params) {
	kproc_t *process = kmalloc(sizeof(kproc_t));
	if(process == NULL) {
		return NULL;
	}

	memset(process, 0, sizeof(kproc_t));

	strncpy(process->name, name, KPROC_NAME_MAX);
	process->pid = get_next_pid();
	
	if(kernel) {
		process->type |= TYPE_KERNEL;
	}

	llist_init(&process->threads);

	process->cwd = fs_root;

	/* TODO: Move to arch */
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
	if(arch_params && arch_params->pgdir) {
		process->arch.cr3 = (uint32_t)arch_params->pgdir;
	} else {
		process->arch.cr3 = (uint32_t)clone_kpagedir();
	}
	if(arch_params) {
		process->arch.ring = arch_params->ring;
	}
#endif

	return process;
}

int proc_add_thread(kproc_t *proc, kthread_t *thread) {
	thread->process = proc;
	if(!thread->tid) {
		if(proc->threads.list) {
			thread->tid = get_next_pid();
		} else {
			thread->tid = proc->pid;
		}
	}
	thread->list_item.data = thread;
	llist_append(&proc->threads, &thread->list_item);

	return 0;
}
