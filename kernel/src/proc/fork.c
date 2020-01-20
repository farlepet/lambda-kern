#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <string.h>

#if  defined(ARCH_X86)
#include <mm/paging.h>
#include <proc/user.h>
#endif

extern lock_t creat_task;

// Assuming SRC is this process
static int proc_copy_stack(struct kproc *dest, const struct kproc *src) {
    kerror(ERR_BOOTINFO, "proc_copy_stack %08X (%08X) -> %08X (%08X)",
        src->stack_beg, (pgdir_get_page_entry((uint32_t *)src->cr3, (void *)(src->stack_beg - 4096)) & (~0xFFF)) + 4096,
        dest->stack_beg, (pgdir_get_page_entry((uint32_t *)dest->cr3, (void *)(dest->stack_beg - 4096)) & (~0xFFF)) + 4096
    );

    // Must be done in 4K increments in case allocated blocks are not sequential

    const size_t stack_size = src->stack_beg - src->stack_end;

    for(size_t i = 0; i < stack_size; i += 0x1000) {
        pgdir_map_page(
            (uint32_t *)src->cr3,
            (void *)(pgdir_get_page_entry((uint32_t *)dest->cr3, (void *)(dest->stack_end + i)) & 0xFFFFF000),
            (void *)(pgdir_get_page_entry((uint32_t *)dest->cr3, (void *)(dest->stack_end + i)) & 0xFFFFF000),
            0x03
        );

        memcpy(
            (void *)(pgdir_get_page_entry((void *)dest->cr3, (void *)(dest->stack_end + i)) & 0xFFFFF000),
            (void *)(src->stack_end + i),
            0x1000
        );
    }

    return 0;
}

static int proc_copy_kernel_stack(struct kproc *dest, const struct kproc *src) {
    kerror(ERR_BOOTINFO, "proc_copy_kernel_stack %08X (%08X) -> %08X (%08X)",
        src->kernel_stack, (pgdir_get_page_entry((uint32_t *)src->cr3, (void *)(src->kernel_stack - 4096)) & (~0xFFF)) + 4096,
        dest->kernel_stack, (pgdir_get_page_entry((uint32_t *)dest->cr3, (void *)(dest->kernel_stack - 4096)) & (~0xFFF)) + 4096
    );
    
    // Kernel stack guaranteed to be linearly mapped:
    for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i += 0x1000) {
        pgdir_map_page((uint32_t *)src->cr3, (void *)(dest->kernel_stack - PROC_KERN_STACK_SIZE + i), (void *)(dest->kernel_stack - PROC_KERN_STACK_SIZE + i), 0x03);
    }

    memcpy((void *)(dest->kernel_stack - PROC_KERN_STACK_SIZE), (void *)(src->kernel_stack - PROC_KERN_STACK_SIZE), PROC_KERN_STACK_SIZE);

    return 0;
}

static int proc_copy_data(struct kproc *dest, const struct kproc *src) {
    kerror(ERR_BOOTINFO, "proc_copy_data");

    struct kproc_mem_map_ent const *pent = src->mmap;
    struct kproc_mem_map_ent *cent = src->mmap;

    // TODO: Split out architecture-dependant portions

    size_t n_ents = 0;
    while(pent != NULL) {
        n_ents++;
        pent = pent->next;
    }
    pent = src->mmap;

    dest->mmap = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent) * n_ents);
    cent = dest->mmap;

    while(pent != NULL) {
        if(pent->next != NULL) {
            cent->next = cent + sizeof(struct kproc_mem_map_ent);
        }

        // Copy values that will be maintained:
        cent->virt_address = pent->virt_address;
        cent->length       = pent->length;

        kerror(ERR_BOOTINFO, "  -> %08X (%d B)", cent->virt_address, cent->length);

        // Allocate new memory:
        cent->phys_address = (uintptr_t)kmalloc(cent->length + 0x1000);

        // Ensure memory has same alignment:
        if((cent->phys_address & 0xFFF) <= (pent->phys_address & 0xFFF)) {
            cent->phys_address = (cent->phys_address & ~0xFFF) | (pent->phys_address & 0xFFF);
        } else {
            cent->phys_address += (0x1000 - ((cent->phys_address & 0xFFF) - (pent->phys_address & 0xFFF)));
        }

        // Map memory (TODO: Optimize):
        for(size_t offset = 0; offset < pent->length; offset += 0x1000) {
            pgdir_map_page((uint32_t *)dest->cr3,
                (void *)(cent->phys_address & ~0xFFF) + offset,
                (void *)(cent->virt_address & ~0xFFF) + offset,
                0x7 // TODO: Check flags of origional map, or add flag to kproc_mem_map_ent to determine type
            );

            // May not be necessary, map memory to itself before we copy:
            map_page(
                (void *)(cent->phys_address & ~0xFFF) + offset,
                (void *)(cent->phys_address & ~0xFFF) + offset,
                0x7 // TODO: Check flags of origional map, or add flag to kproc_mem_map_ent to determine type
            );
        }

        // Copy data:
        memcpy((void *)cent->phys_address, (void *)pent->virt_address, pent->length);

        // Move on to next memory map entry:
        cent = cent->next;
        pent = pent->next;
    }

    return 0;
}


static int __no_inline fork_clone_process(uint32_t child_idx, uint32_t parent_idx) {
    // TODO: Sort out X86-specific bits!
    // TODO: Clean up!

    struct kproc *child  = &procs[child_idx];
    struct kproc *parent = &procs[parent_idx];

    uint32_t i = 0;
    for(; i < MAX_CHILDREN; i++) {
        if(!parent->children[i]) {
            parent->children[i] = child_idx;
            break;
        }
    }
    if(i == MAX_CHILDREN) {
        kerror(ERR_SMERR, "mtask:add_task: Process %d has run out of children slots", parent->pid);
        unlock(&creat_task);
        return -1;
    }

    int kernel = (current_pid < 0);

    // Doing a memcpy might be more efficient removing some instructions, but it
    // may also introduce bugs/security flaws if certain info isn't cleared properly.
    memset(child, 0, sizeof(struct kproc));

    memcpy(child->name, parent->name, strlen(parent->name));

    child->pid = get_next_pid(kernel);

    child->uid  = parent->uid;
    child->gid  = parent->gid;

    child->type = TYPE_VALID | TYPE_RANONCE;
    if(kernel) child->type |= TYPE_KERNEL;

    child->prio = parent->prio;

    uint32_t *pagedir = clone_pagedir_full((void *)parent->cr3);

    child->ring       = parent->ring;
    //child->eip        = parent->eip;
    child->entrypoint = parent->entrypoint;
    child->cr3        = (uint32_t)pagedir;

    uint32_t stack_size = parent->stack_beg - parent->stack_end;
    uint32_t /*stack_begin, */virt_stack_begin;

    if(!kernel) virt_stack_begin = 0xFF000000;
    else        virt_stack_begin = 0x7F000000;

    //child->esp = parent->esp;
    child->ebp = parent->ebp;

    proc_create_stack(child, stack_size, virt_stack_begin, kernel);
    proc_create_kernel_stack(child);

    // POPAD: 8 DWORDS, IRETD: 5 DWORDS
    child->esp = child->kernel_stack - 52;
    child->eip = (uint32_t)return_from_fork;

    proc_copy_stack(child, parent);
    proc_copy_kernel_stack(child, parent);

    proc_copy_data(child, parent);

    kerror(ERR_INFO, "IRETD_VALS: %08X %08X %08X %08X %08X",
        *(uint32_t *)(child->kernel_stack - 4), *(uint32_t *)(child->kernel_stack - 8), *(uint32_t *)(child->kernel_stack - 12), *(uint32_t *)(child->kernel_stack - 16),
        *(uint32_t *)(child->kernel_stack - 20)
    );

    kerror(ERR_INFO, "POPAD_VALS: %08X %08X %08X %08X %08X %08X %08X %08X",
        *(uint32_t *)(child->kernel_stack - 24), *(uint32_t *)(child->kernel_stack - 28), *(uint32_t *)(child->kernel_stack - 32),
        *(uint32_t *)(child->kernel_stack - 36), *(uint32_t *)(child->kernel_stack - 40), *(uint32_t *)(child->kernel_stack - 44), *(uint32_t *)(child->kernel_stack - 48),
        *(uint32_t *)(child->kernel_stack - 52)
    );


    kerror(ERR_INFO, " -- eip: %08X esp: %08X ebp: %08X", child->eip, child->esp, child->ebp);

    // Set up message buffer
    child->messages.size  = MSG_BUFF_SIZE;
    child->messages.buff  = child->msg_buff;

    // Copy open file descriptors:
    memcpy(child->open_files, parent->open_files, sizeof(child->open_files));
    memcpy(child->file_position, parent->file_position, sizeof(child->file_position));

    // Set working directory:
    child->cwd = parent->cwd;

    return 0;
}


int fork(void) {
    if(!tasking) {
        kerror(ERR_MEDERR, "mtask:fork: Attempted to fork before multitasking enabled!!!");
        return -1;
    }

    lock(&creat_task);
	
    kerror(ERR_INFO, "mtask:fork()");

    int p = get_next_open_proc();
    if(p == -1) {
        kerror(ERR_MEDERR, "mtask:fork: Too many processes, could not create new one.");
        return -1;
    }

    struct kproc *child  = &procs[p];

    fork_clone_process(p, proc_by_pid(current_pid));

    //child->eip = (uintptr_t)return_from_syscall;

    kerror(ERR_INFO, " -- Child Stack: %08X %08X", child->esp, child->ebp);

    child->type |= TYPE_RUNNABLE;

    unlock(&creat_task);

    return child->pid;
}