#include <string.h>

#include <arch/proc/tasking.h>
#include <arch/proc/user.h>
#include <arch/intr/int.h>
#include <arch/mm/paging.h>
#include <arch/mm/gdt.h>

#include <proc/atomic/lock.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <intr/intr.h>
#include <err/error.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <types.h>

extern lock_t creat_task; // From proc/mtask.c

extern void *get_eip(void);  //!< Get the EIP value of the instruction after the call to this function

static void _sched_int(intr_handler_hand_t *hdlr) {
    (void)hdlr;
    do_task_switch();
}

/* @todo Possibly make this dynamically allocated */
static intr_handler_hand_t _sched_int_hdlr = {
    .callback = _sched_int,
    .data     = NULL
};

void arch_multitasking_init(void) {
    interrupt_attach(INTR_SCHED, &_sched_int_hdlr);
}

int arch_proc_create_stack(kthread_t *thread) {
    if(!SAFETY_CHECK(thread->process)) {
        kpanic("arch_proc_create_stack: Thread has no associated process!");
    }

    int kernel = thread->process->domain != PROC_DOMAIN_USERSPACE;
    
    uint32_t virt_stack_begin = 0x7F000000;
    
    /* @todo Find better scheme to separate thread stacks. */
    int tpos = llist_get_position(&thread->process->threads, &thread->list_item);
#ifdef CONFIG_PROC_STACK_GUARD
    virt_stack_begin -= tpos * (CONFIG_PROC_USER_STACK_SIZE_MAX + 0x2000);
#else
    virt_stack_begin -= tpos * CONFIG_PROC_USER_STACK_SIZE_MAX;
#endif

    uintptr_t stack_begin = (uintptr_t)kmamalloc(thread->stack_size, 4096);

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "arch_proc_create_stack [size: %d] [end: %08X, beg: %08X]",
        thread->stack_size, stack_begin, stack_begin + thread->stack_size
    );

    // Map stack memory
    mmu_map_table(thread->process->mmu_table, virt_stack_begin, stack_begin, thread->stack_size,
                  kernel ? (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL) :
                           (MMU_FLAG_READ | MMU_FLAG_WRITE));
    
    thread->arch.stack_user.size  = thread->stack_size;
    thread->arch.stack_user.begin = virt_stack_begin + thread->stack_size;

#ifdef CONFIG_PROC_STACK_GUARD
    map_page(0, (void *)thread->arch.stack_user.begin, 0);
    map_page(0, (void *)(thread->arch.stack_user.begin - thread->arch.stack_user.size - 0x1000), 0);
#endif

    thread->arch.ebp = thread->arch.stack_user.begin;

    return 0;
}

int arch_proc_create_kernel_stack(kthread_t *thread) {
    thread->arch.stack_kern.size  = CONFIG_PROC_KERN_STACK_SIZE;
    thread->arch.stack_kern.begin = (uint32_t)kmamalloc(thread->arch.stack_kern.size, 4096);

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "arch_proc_create_kernel_stack [size: %d] [end: %08X, beg: %08X]",
        CONFIG_PROC_KERN_STACK_SIZE, thread->arch.stack_kern.begin, thread->arch.stack_kern.begin + thread->arch.stack_kern.size
    );

    mmu_map_table(thread->process->mmu_table, thread->arch.stack_kern.begin, thread->arch.stack_kern.begin, CONFIG_PROC_KERN_STACK_SIZE,
                  (MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL));

    thread->arch.stack_kern.begin += thread->arch.stack_kern.size;

    return 0;
}

__noreturn
static void _thread_entrypoint(void) {
    kthread_t *curr_thread = sched_get_curr_thread(0);
    if(curr_thread == NULL) {
        kpanic("_proc_entrypoint: Thread is NULL!");
    }

    kproc_t *curr_proc = curr_thread->process;
    if(curr_proc == NULL) {
        kpanic("_proc_entrypoint: Process is NULL!");
    }
    if(curr_thread->entrypoint == 0) {
        kpanic("_proc_entrypoint: Entrypoint is NULL!");
    }

    if(!(curr_thread->flags & KTHREAD_FLAG_STACKSETUP)) {
        if(curr_proc->domain != PROC_DOMAIN_USERSPACE) {
            STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->thread_data);
        } else {
            /* Empty args and env for CRT0 w/ user applications */
            STACK_PUSH(curr_thread->arch.stack_entry, NULL);                                   /* ENVP */
            STACK_PUSH(curr_thread->arch.stack_entry, NULL);                                   /* ARGV */
            STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->arch.stack_user.begin - 4); /* ENVP ptr */
            STACK_PUSH(curr_thread->arch.stack_entry, curr_thread->arch.stack_user.begin - 8); /* ARGV ptr */
            STACK_PUSH(curr_thread->arch.stack_entry, 0);                                      /* ARGC */
        }
    }

    uint8_t ring = (curr_proc->domain == PROC_DOMAIN_KERNEL) ? 0 :
                   (curr_proc->domain == PROC_DOMAIN_MODULE) ? 0 : 3;

    enter_ring_newstack(ring, (void *)curr_thread->entrypoint, (void *)curr_thread->arch.stack_entry);
    
    exit(1);
}

int arch_setup_thread(kthread_t *thread) {
    proc_create_stack(thread);
    proc_create_kernel_stack(thread);
    
    thread->arch.eip         = (uint32_t)_thread_entrypoint;
    thread->arch.esp         = thread->arch.stack_kern.begin;
    thread->arch.stack_entry = thread->arch.stack_user.begin;

    kdebug(DEBUGSRC_PROC, ERR_TRACE, "arch_setup_thread EIP: %08X CR3: %08X ESP: %08X", thread->arch.eip, thread->process->mmu_table, thread->arch.esp);

    return 0;
}

int arch_setup_process(kproc_t __unused *proc) {
    return 0;
}

void arch_setup_user_stack(kthread_t *thread, int argc, char **argv, char **envp) {
    thread->arch.esp = thread->arch.ebp;

    STACK_PUSH(thread->arch.esp, envp);
    STACK_PUSH(thread->arch.esp, argv);
    STACK_PUSH(thread->arch.esp, argc);

    thread->arch.stack_entry = thread->arch.esp;

    thread->flags |= KTHREAD_FLAG_STACKSETUP;
}

int arch_postfork_setup(const kthread_t *parent, kthread_t *child) {
    child->arch.ebp = parent->arch.ebp;

    // POPAD: 8 DWORDS, IRETD: 5 DWORDS
    child->arch.esp = child->arch.stack_kern.begin - 52;
    child->arch.eip = (uint32_t)return_from_fork;

    x86_iret_regs_t  *iret_stack  = (x86_iret_regs_t *)(child->arch.stack_kern.begin - sizeof(x86_iret_regs_t));
    x86_pusha_regs_t *pusha_stack = (x86_pusha_regs_t *)((uintptr_t)iret_stack - sizeof(x86_pusha_regs_t));

    memcpy(iret_stack,  parent->arch.syscall_regs.iret,  sizeof(x86_iret_regs_t));
    memcpy(pusha_stack, parent->arch.syscall_regs.pusha, sizeof(x86_pusha_regs_t));

    uintptr_t syscall_args_virt = pusha_stack->ebx;

    /* Return 0 indicating child process */
    uint32_t zero = 0;
    if(mmu_write_data(child->process->mmu_table, syscall_args_virt, &zero, 4)) {
        return -1;
    }

    kdebug(DEBUGSRC_PROC, ERR_TRACE, " -- eip: %08X esp: %08X ebp: %08X cr3: %p", child->arch.eip, child->arch.esp, child->arch.ebp, child->process->mmu_table);

    return 0;
}

__hot void do_task_switch(void) {
    /* TODO: Select proper CPU */
    kthread_t *thread = sched_get_curr_thread(0);
    if(!thread) {
        return;
    }

    uint32_t esp, ebp, eip, cr3;
    asm volatile ("mov %%esp, %0" : "=r" (esp));
    asm volatile ("mov %%ebp, %0" : "=r" (ebp));
    eip = (uint32_t)get_eip();

    if(eip == 0xFFFFFFFF) {
        return;
    }

    if(thread->flags & KTHREAD_FLAG_RANONCE) {
        thread->arch.esp = esp;
        thread->arch.ebp = ebp;
        thread->arch.eip = eip;
    }

    kdebug(DEBUGSRC_PROC, ERR_ALL, "-TID: %d | PC: %08X | SP: %08X | CR3: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->process->mmu_table, thread->name);

    // Switch to next process here...
    thread = sched_next_process(0);

    kdebug(DEBUGSRC_PROC, ERR_ALL, "+TID: %d | PC: %08X | SP: %08X | CR3: %08X | NAME: %s", thread->tid, thread->arch.eip, thread->arch.esp, thread->process->mmu_table, thread->name);

    thread->flags |= KTHREAD_FLAG_RANONCE;
    if (!thread->arch.stack_kern.begin) {   
        kpanic("do_task_switch: No kernel stack set for thread!");
    }
    tss_set_kern_stack(thread->arch.stack_kern.begin);

    esp = thread->arch.esp;
    ebp = thread->arch.ebp;
    eip = thread->arch.eip;
    cr3 = (uint32_t)thread->process->mmu_table;

    asm volatile("mov %0, %%ebx\n"
                 "mov %1, %%esp\n"
                 "mov %2, %%ebp\n"
                 "mov %3, %%cr3\n"
                 "mov $0xFFFFFFFF, %%eax\n"
                 "sti\n"
                 "jmp *%%ebx"
                 : : "r" (eip), "r" (esp), "r" (ebp), "r" (cr3)
                 : "%ebx", /*"%esp", */"%eax");
}
