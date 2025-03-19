#include <errno.h>
#include <string.h>

#include <proc/mtask.h>
#include <proc/thread.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <fs/fs.h>


int proc_add_file(kproc_t *proc, kfile_hand_t *file) {
    for(int i = 0; i < MAX_OPEN_FILES; i++) {
        if(proc->open_files[i] == NULL) {
            proc->open_files[i] = file;
            proc->file_position[i] = 0;
            return i;
        }
    }

    return -ENFILE;
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

    return -EUNSPEC;
}

kproc_t *proc_create(char *name, int domain, mmu_table_t *mmu_table) {
    kproc_t *process = kmalloc(sizeof(kproc_t));
    if(process == NULL) {
        return NULL;
    }

    memset(process, 0, sizeof(kproc_t));

    strncpy(process->name, name, KPROC_NAME_MAX);
    process->pid = get_next_pid();
    
    process->domain = domain;

    llist_init(&process->threads);

    process->cwd = fs_get_root();

    if(mmu_table) {
        process->mmu_table = mmu_table;
    } else {
        process->mmu_table = mmu_clone_table(mmu_get_kernel_table());
    }

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

int proc_destroy(kproc_t *proc) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "proc_destroy: %d, %s", proc->pid, proc->name);

    mtask_remove_proc(proc);

    if(proc->mmu_table) {
        /* TODO: MMU table doesn't necessarially need to be a single allocation */
        kfree(proc->mmu_table);
    }

    if(proc->symbols) {
        kfree(proc->symbols);
    }

    if(proc->elf_data) {
        kfree(proc->elf_data);
    }

    /* TODO: All allocations should be tracked in mmap, to simplify process
     * destruction */
    if(proc->mmap) {
    /*while(proc->mmap) {
        struct kproc_mem_map_ent *ent = proc->mmap;
        proc->mmap = proc->mmap->next; */
        kfree(proc->mmap);
        /* @todo: Sometimes mmap is a single allocated block, sometimes it is a
         * set of allocated blocks. */
    }

    while(proc->threads.list) {
        kthread_t *thread = proc->threads.list->data;
        llist_remove(&proc->threads, proc->threads.list);

        thread_destroy(thread);
    }

    kfree(proc);

    return 0;
}
