#include <arch/proc/stack.h>
#include <arch/mm/paging.h>
#include <arch/proc/user.h>

#include <err/error.h>
#include <string.h>

int proc_copy_stack(kthread_t *dest, const kthread_t *src) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE,  "proc_copy_stack %08X (%08X) -> %08X (%08X)",
        src->arch.stack_user.begin, (pgdir_get_page_entry((uint32_t *)src->process->arch.cr3, (void *)(src->arch.stack_user.begin - 4096)) & (~0xFFF)) + 4096,
        dest->arch.stack_user.begin, (pgdir_get_page_entry((uint32_t *)dest->process->arch.cr3, (void *)(dest->arch.stack_user.begin - 4096)) & (~0xFFF)) + 4096
    );

    // Must be done in 4K increments in case allocated blocks are not sequential

    for(size_t i = 0; i < src->arch.stack_user.size; i += 0x1000) {
        uint32_t end_src  = src->arch.stack_user.begin  - src->arch.stack_user.begin;
        uint32_t end_dest = dest->arch.stack_user.begin - dest->arch.stack_user.begin;

        /* TODO: It would probably be helpful to create a function that copies data
         * from one directory to another. */

        pgdir_map_page(
            (uint32_t *)src->process->arch.cr3,
            (void *)(pgdir_get_page_entry((uint32_t *)dest->process->arch.cr3, (void *)(end_dest + i)) & 0xFFFFF000),
            (void *)(pgdir_get_page_entry((uint32_t *)dest->process->arch.cr3, (void *)(end_dest + i)) & 0xFFFFF000),
            0x03
        );

        memcpy(
            (void *)(pgdir_get_page_entry((void *)dest->process->arch.cr3, (void *)(end_dest + i)) & 0xFFFFF000),
            (void *)(end_src + i),
            0x1000
        );
    }

    return 0;
}

int proc_copy_kernel_stack(kthread_t *dest, const kthread_t *src) {
    kdebug(DEBUGSRC_PROC, ERR_TRACE, "proc_copy_kernel_stack %08X (%08X) -> %08X (%08X)",
        src->arch.stack_kern.begin, (pgdir_get_page_entry((uint32_t *)src->process->arch.cr3, (void *)(src->arch.stack_kern.begin - 4096)) & (~0xFFF)) + 4096,
        dest->arch.stack_kern.begin, (pgdir_get_page_entry((uint32_t *)dest->process->arch.cr3, (void *)(dest->arch.stack_kern.begin - 4096)) & (~0xFFF)) + 4096
    );
    
    // Kernel stack guaranteed to be linearly mapped:
    for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i += 0x1000) {
        pgdir_map_page((uint32_t *)src->process->arch.cr3,
                       (void *)(dest->arch.stack_kern.begin - PROC_KERN_STACK_SIZE + i),
                       (void *)(dest->arch.stack_kern.begin - PROC_KERN_STACK_SIZE + i), 0x03);
    }

    memcpy((void *)(dest->arch.stack_kern.begin - PROC_KERN_STACK_SIZE),
           (void *)(src->arch.stack_kern.begin - PROC_KERN_STACK_SIZE), PROC_KERN_STACK_SIZE);

    return 0;
}