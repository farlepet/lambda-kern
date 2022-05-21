#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/proc/user.h>
#  include <arch/proc/stack.h>
#endif


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
static int proc_copy_data(kthread_t *dest, const kthread_t *src) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "proc_copy_data");

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    struct kproc_mem_map_ent const *pent = src->process->mmap;
    struct kproc_mem_map_ent *cent;

    // TODO: Split out architecture-dependant portions (e.g. memory mapping)

    size_t n_ents = 0;
    while(pent != NULL) {
        n_ents++;
        pent = pent->next;
    }
    pent = src->process->mmap;

    dest->process->mmap = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent) * n_ents);
    cent = dest->process->mmap;

    while(pent != NULL) {
        if(pent->next != NULL) {
            cent->next = cent + 1;
        }

        // Copy values that will be maintained:
        cent->virt_address = pent->virt_address;
        cent->length       = pent->length;

        kdebug(DEBUGSRC_PROC, ERR_TRACE, "  -> %08X (%d B)", cent->virt_address, cent->length);

        // Allocate new memory:
        cent->phys_address = (uintptr_t)kmalloc(cent->length + 0x1000);

        // Ensure memory has same alignment:
        if((cent->phys_address & 0xFFF) <= (pent->phys_address & 0xFFF)) {
            cent->phys_address = (cent->phys_address & ~0xFFF) | (pent->phys_address & 0xFFF);
        } else {
            cent->phys_address += (0x1000 - ((cent->phys_address & 0xFFF) - (pent->phys_address & 0xFFF)));
        }


        /* Map memory in new process */
        mmu_map_table(dest->process->mmu_table, cent->virt_address, cent->phys_address, pent->length,
                      (MMU_FLAG_READ | MMU_FLAG_READ) /* TODO: Dynamically determine flags */);

        mmu_copy_data(dest->process->mmu_table, cent->virt_address,
                      src->process->mmu_table,  pent->virt_address,
                      pent->length);

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
        kdebug(DEBUGSRC_PROC, ERR_DEBUG, "mtask:add_task: Process %d has run out of children slots", parent->pid);
        unlock(&creat_task);
        return -1;
    }

    int kernel = (parent->type & TYPE_KERNEL);


    memcpy(child->name, parent->name, strlen(parent->name));

	memset(child->children, 0xFF, sizeof(child->children));

    kthread_t *cthread = (kthread_t *)child->threads.list->data;
    kthread_t *pthread = mtask_get_curr_thread();

    cthread->process = child;
    child->pid   = get_next_pid();
    cthread->tid = child->pid;

    child->uid  = parent->uid;
    child->gid  = parent->gid;

    if(kernel) child->type |= TYPE_KERNEL;
    cthread->flags |= KTHREAD_FLAG_RANONCE;

    cthread->prio = pthread->prio;

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    child->arch.ring    = parent->arch.ring;
    cthread->entrypoint = pthread->entrypoint;
    child->mmu_table    = mmu_clone_table(parent->mmu_table);

    cthread->stack_size = pthread->arch.stack_user.size;
    proc_create_stack(cthread);
    proc_create_kernel_stack(cthread);

    cthread->arch.ebp = pthread->arch.ebp;
    
    // POPAD: 8 DWORDS, IRETD: 5 DWORDS
    cthread->arch.esp = cthread->arch.stack_kern.begin - 52;
    cthread->arch.eip = (uint32_t)return_from_fork;

    proc_copy_stack(cthread, pthread);
    //proc_copy_kernel_stack(child, parent);

    proc_copy_data(cthread, pthread);

    arch_iret_regs_t  *iret_stack  = (arch_iret_regs_t *)(cthread->arch.stack_kern.begin - sizeof(arch_iret_regs_t));
    arch_pusha_regs_t *pusha_stack = (arch_pusha_regs_t *)((uintptr_t)iret_stack - sizeof(arch_pusha_regs_t));
    
    memcpy(iret_stack,  pthread->arch.syscall_regs.iret,  sizeof(arch_iret_regs_t));
    memcpy(pusha_stack, pthread->arch.syscall_regs.pusha, sizeof(arch_pusha_regs_t));

#  if 0
    kdebug(DEBUGSRC_PROC, "IRET_STACK (%08X):", iret_stack);
    for(size_t i = 0; i < 5; i++) {
        kdebug(DEBUGSRC_PROC, "    %08X", ((uint32_t *)iret_stack)[i]);
    }
    kdebug(DEBUGSRC_PROC, "PUSHA_STACK (%08X):", pusha_stack);
    for(size_t i = 0; i < 8; i++) {
        kdebug(DEBUGSRC_PROC, "    %08X", ((uint32_t *)pusha_stack)[i]);
    }
#  endif


    uint32_t *syscall_args_virt = (uint32_t *)pusha_stack->ebx;
    uint32_t *syscall_args_phys = 0;
    mmu_map_get_table(child->mmu_table, (uintptr_t)syscall_args_virt, (uintptr_t *)&syscall_args_phys);

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "ARGS_LOC: %08X -> %08X -> %08X", syscall_args_virt, syscall_args_phys, *(uint32_t *)syscall_args_phys);
    syscall_args_phys[0] = 0; // <- Return 0 indicating child process



    kdebug(DEBUGSRC_PROC, ERR_TRACE, " -- eip: %08X esp: %08X ebp: %08X cr3: %p", cthread->arch.eip, cthread->arch.esp, cthread->arch.ebp, child->mmu_table);
#else
    /* TODO */
    proc_copy_data(cthread, pthread);
#endif

    // Copy open file descriptors:
    memcpy(child->open_files, parent->open_files, sizeof(child->open_files));
    memcpy(child->file_position, parent->file_position, sizeof(child->file_position));

    // Set working directory:
    child->cwd = parent->cwd;

    return 0;
}


int fork(void) {
    kthread_t *thread = mtask_get_curr_thread();
    if(!thread) {
        kpanic("mtask:fork: Attempted to fork before multitasking enabled!!!");
    }
    kproc_t *proc = thread->process;

    lock(&creat_task);
	
    kdebug(DEBUGSRC_PROC, ERR_DEBUG, "mtask:fork()");

    struct kproc *child = (struct kproc *)kmalloc(sizeof(struct kproc));
    // Doing a memcpy might be more efficient removing some instructions, but it
    // may also introduce bugs/security flaws if certain info isn't cleared properly.
    memset(child, 0, sizeof(struct kproc));
    child->parent = proc->pid;
    
    kthread_t *cthread = (kthread_t *)kmalloc(sizeof(kthread_t));
    if(cthread == NULL) {
        kpanic("kthread_create: Ran out of memory attempting to allocate thread!");
    }
    memset(cthread, 0, sizeof(kthread_t));
	
    llist_init(&child->threads);
    proc_add_thread(child, cthread);

    fork_clone_process(child, proc);

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    kdebug(DEBUGSRC_PROC, ERR_TRACE, " -- Child Stack: %08X %08X", cthread->arch.esp, cthread->arch.ebp);
#endif

    proc_add_child(proc, child);

    child->type |= TYPE_RUNNABLE;
    cthread->flags |= KTHREAD_FLAG_RUNNABLE;

    mtask_insert_proc(child);
    sched_enqueue_thread(cthread);

    unlock(&creat_task);

    return child->pid;
}
