#include <proc/mtask.h>
#include <err/error.h>
#include <string.h>

#if  defined(ARCH_X86)
#include <mm/paging.h>
#endif

extern lock_t creat_task;

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

    uint32_t *pagedir = clone_pagedir((void *)parent->cr3);

    child->ring       = parent->ring;
    child->eip        = parent->eip;
    child->entrypoint = parent->entrypoint;
    child->cr3        = (uint32_t)pagedir;

    uint32_t stack_size = parent->stack_beg - parent->stack_end;
    uint32_t /*stack_begin, */virt_stack_begin;
    if(!kernel) virt_stack_begin = 0xFF000000;
    else        virt_stack_begin = 0x7F000000;

    child->esp = parent->esp;
    child->ebp = parent->ebp;

    proc_create_stack(child, stack_size, virt_stack_begin, kernel);
    proc_create_kernel_stack(child);

    memcpy((void *)pgdir_get_page_entry((uint32_t *)child->cr3, (void *)(child->kernel_stack - PROC_KERN_STACK_SIZE)), (void *)pgdir_get_page_entry((uint32_t *)child->cr3, (void *)(parent->kernel_stack - PROC_KERN_STACK_SIZE)), PROC_KERN_STACK_SIZE);
	
    // TODO: Maybe make this more effecient if stack size is large? i.e. only copy used portion?
    memcpy((void *)pgdir_get_page_entry((uint32_t *)child->cr3, (void *)virt_stack_begin), (void *)pgdir_get_page_entry((uint32_t *)parent->cr3, (void *)virt_stack_begin), stack_size);
    //memcpy((void *)pgdir_get_page_entry((uint32_t *)child->cr3, (void *)virt_stack_begin), (void *)parent->stack_end, stack_size);
    //memcpy((void *)child->stack_end, (void *)parent->stack_end, stack_size);

    kerror(ERR_INFO, " -- eip: %08X esp: %08X ebp: %08X", child->eip, child->esp, child->ebp);

    // TODO: Copy other process-mapped memory!

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

    child->eip = (u32)return_from_syscall;

    kerror(ERR_INFO, " -- Child Stack: %08X %08X", child->esp, child->ebp);

    child->type |= TYPE_RUNNABLE;

    unlock(&creat_task);

    return child->pid;
}