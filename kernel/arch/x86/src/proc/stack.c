#include <arch/proc/stack.h>
#include <arch/mm/paging.h>
#include <arch/proc/user.h>

#include <err/error.h>
#include <string.h>

int proc_copy_stack(kthread_t *dest, const kthread_t *src) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE,  "proc_copy_stack %08X (%08X) -> %08X (%08X)",
        src->arch.stack_user.begin, (pgdir_get_page_entry((uint32_t *)src->process->mmu_table, (void *)(src->arch.stack_user.begin - 4096)) & (~0xFFF)) + 4096,
        dest->arch.stack_user.begin, (pgdir_get_page_entry((uint32_t *)dest->process->mmu_table, (void *)(dest->arch.stack_user.begin - 4096)) & (~0xFFF)) + 4096
    );

    uintptr_t end_src  = src->arch.stack_user.begin  - src->arch.stack_user.size;
    uintptr_t end_dest = dest->arch.stack_user.begin - dest->arch.stack_user.size;

    mmu_copy_data(dest->process->mmu_table, end_dest,
                  src->process->mmu_table,  end_src,
                  src->arch.stack_user.size);

    return 0;
}

int proc_copy_kernel_stack(kthread_t *dest, const kthread_t *src) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "proc_copy_kernel_stack %08X (%08X) -> %08X (%08X)",
        src->arch.stack_kern.begin, (pgdir_get_page_entry((uint32_t *)src->process->mmu_table, (void *)(src->arch.stack_kern.begin - 4096)) & (~0xFFF)) + 4096,
        dest->arch.stack_kern.begin, (pgdir_get_page_entry((uint32_t *)dest->process->mmu_table, (void *)(dest->arch.stack_kern.begin - 4096)) & (~0xFFF)) + 4096
    );

    mmu_copy_data(dest->process->mmu_table, (dest->arch.stack_kern.begin - CONFIG_PROC_KERN_STACK_SIZE),
                  src->process->mmu_table,  (src->arch.stack_kern.begin  - CONFIG_PROC_KERN_STACK_SIZE),
                  CONFIG_PROC_KERN_STACK_SIZE);

    return 0;
}
