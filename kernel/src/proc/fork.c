#include <proc/mtask.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <string.h>

#if  defined(ARCH_X86)
#  include <arch/proc/user.h>
#  include <arch/mm/paging.h>
#  include <arch/proc/stack.h>
#endif

extern lock_t creat_task;


/**
 * @brief Copies process-allocated data (including program data) from source
 * process to child process
 * 
 * Note: The source process is assumed to be the current process
 * 
 * @param dest Destination process
 * @param src Source process
 * @return int 0 on success
 */
static int proc_copy_data(struct kproc *dest, const struct kproc *src) {
    kerror(ERR_BOOTINFO, "proc_copy_data");

#if defined(ARCH_X86)
    struct kproc_mem_map_ent const *pent = src->mmap;
    struct kproc_mem_map_ent *cent;

    // TODO: Split out architecture-dependant portions (e.g. memory mapping)

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
            pgdir_map_page((uint32_t *)dest->arch.cr3,
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
#else
    /* TODO */
    (void)dest;
    (void)src;
#endif

    return 0;
}

/**
 * @brief Clone process as part of a fork() call
 * 
 * @param child_idx Index of child process in process array
 * @param parent_idx Index of parent process in process array
 * @return int 0 on success 
 */
static int __no_inline fork_clone_process(struct kproc *child, struct kproc *parent) {
    // TODO: Sort out X86-specific bits!
    // TODO: Clean up!

    if(proc_add_child(parent, child)) {
        kerror(ERR_SMERR, "mtask:add_task: Process %d has run out of children slots", parent->pid);
        unlock(&creat_task);
        return -1;
    }

    int kernel = (parent->type & TYPE_KERNEL);

    // Doing a memcpy might be more efficient removing some instructions, but it
    // may also introduce bugs/security flaws if certain info isn't cleared properly.
    memset(child, 0, sizeof(struct kproc));

    memcpy(child->name, parent->name, strlen(parent->name));

	memset(child->children, 0xFF, sizeof(child->children));
 
    child->pid = get_next_pid();

    child->uid  = parent->uid;
    child->gid  = parent->gid;

    child->type = TYPE_VALID | TYPE_RANONCE;
    if(kernel) child->type |= TYPE_KERNEL;

    child->prio = parent->prio;

#if defined(ARCH_X86)
    child->arch.ring  = parent->arch.ring;
    child->entrypoint = parent->entrypoint;
    child->arch.cr3   = (uint32_t)clone_pagedir_full((void *)parent->arch.cr3);

    uint32_t stack_size = parent->arch.stack_beg - parent->arch.stack_end;
    uint32_t virt_stack_begin;

    if(!kernel) virt_stack_begin = 0xFF000000;
    else        virt_stack_begin = 0x7F000000;


    proc_create_stack(child, stack_size, virt_stack_begin, kernel);
    proc_create_kernel_stack(child);

    child->arch.ebp = parent->arch.ebp;
    
    // POPAD: 8 DWORDS, IRETD: 5 DWORDS
    child->arch.esp = child->arch.kernel_stack - 52;
    child->arch.eip = (uint32_t)return_from_fork;

    proc_copy_stack(child, parent);
    //proc_copy_kernel_stack(child, parent);

    proc_copy_data(child, parent);

    arch_iret_regs_t  *iret_stack  = (arch_iret_regs_t *)(child->arch.kernel_stack - sizeof(arch_iret_regs_t));
    arch_pusha_regs_t *pusha_stack = (arch_pusha_regs_t *)((uintptr_t)iret_stack - sizeof(arch_pusha_regs_t));
    
    memcpy(iret_stack,  parent->arch.syscall_regs.iret,  sizeof(arch_iret_regs_t));
    memcpy(pusha_stack, parent->arch.syscall_regs.pusha, sizeof(arch_pusha_regs_t));
    
    kerror(ERR_INFO, "IRET_STACK (%08X):", iret_stack);
    for(size_t i = 0; i < 5; i++) {
        kerror(ERR_INFO, "    %08X", ((uint32_t *)iret_stack)[i]);
    }
    kerror(ERR_INFO, "PUSHA_STACK (%08X):", pusha_stack);
    for(size_t i = 0; i < 8; i++) {
        kerror(ERR_INFO, "    %08X", ((uint32_t *)pusha_stack)[i]);
    }


    uint32_t *syscall_args_virt = (uint32_t *)pusha_stack->ebx;
    uint32_t *syscall_args_phys = (uint32_t *)pgdir_get_phys_addr((uint32_t *)child->arch.cr3, syscall_args_virt);
    kerror(ERR_INFO, "ARGS_LOC: %08X -> %08X -> %08X", syscall_args_virt, syscall_args_phys, *(uint32_t *)syscall_args_phys);
    syscall_args_phys[0] = 0; // <- Return 0 indicating child process



    kerror(ERR_INFO, " -- eip: %08X esp: %08X ebp: %08X cr3: %08X", child->arch.eip, child->arch.esp, child->arch.ebp, child->arch.cr3);
#else
    /* TODO */
    proc_copy_data(child, parent);
#endif

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
    if(!curr_proc) {
        kerror(ERR_MEDERR, "mtask:fork: Attempted to fork before multitasking enabled!!!");
        return -1;
    }

    lock(&creat_task);
	
    kerror(ERR_INFO, "mtask:fork()");

    struct kproc *child = (struct kproc *)kmalloc(sizeof(struct kproc));

    fork_clone_process(child, curr_proc);

#if defined(ARCH_X86)
    kerror(ERR_INFO, " -- Child Stack: %08X %08X", child->arch.esp, child->arch.ebp);
#endif

    child->parent = curr_proc->pid;
    proc_add_child(curr_proc, child);

    child->type |= TYPE_RUNNABLE;

    mtask_insert_proc(child);

    unlock(&creat_task);

    return child->pid;
}
