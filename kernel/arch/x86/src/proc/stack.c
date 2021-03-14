#include <arch/proc/stack.h>
#include <arch/mm/paging.h>
#include <arch/proc/user.h>

#include <err/error.h>
#include <string.h>

int proc_copy_stack(struct kproc *dest, const struct kproc *src) {
    kdebug(DEBUGSRC_PROC, "proc_copy_stack %08X (%08X) -> %08X (%08X)",
        src->arch.stack_beg, (pgdir_get_page_entry((uint32_t *)src->arch.cr3, (void *)(src->arch.stack_beg - 4096)) & (~0xFFF)) + 4096,
        dest->arch.stack_beg, (pgdir_get_page_entry((uint32_t *)dest->arch.cr3, (void *)(dest->arch.stack_beg - 4096)) & (~0xFFF)) + 4096
    );

    // Must be done in 4K increments in case allocated blocks are not sequential

    const size_t stack_size = src->arch.stack_beg - src->arch.stack_end;

    for(size_t i = 0; i < stack_size; i += 0x1000) {
        pgdir_map_page(
            (uint32_t *)src->arch.cr3,
            (void *)(pgdir_get_page_entry((uint32_t *)dest->arch.cr3, (void *)(dest->arch.stack_end + i)) & 0xFFFFF000),
            (void *)(pgdir_get_page_entry((uint32_t *)dest->arch.cr3, (void *)(dest->arch.stack_end + i)) & 0xFFFFF000),
            0x03
        );

        memcpy(
            (void *)(pgdir_get_page_entry((void *)dest->arch.cr3, (void *)(dest->arch.stack_end + i)) & 0xFFFFF000),
            (void *)(src->arch.stack_end + i),
            0x1000
        );
    }

    return 0;
}

int proc_copy_kernel_stack(struct kproc *dest, const struct kproc *src) {
    kdebug(DEBUGSRC_PROC, "proc_copy_kernel_stack %08X (%08X) -> %08X (%08X)",
        src->arch.kernel_stack, (pgdir_get_page_entry((uint32_t *)src->arch.cr3, (void *)(src->arch.kernel_stack - 4096)) & (~0xFFF)) + 4096,
        dest->arch.kernel_stack, (pgdir_get_page_entry((uint32_t *)dest->arch.cr3, (void *)(dest->arch.kernel_stack - 4096)) & (~0xFFF)) + 4096
    );
    
    // Kernel stack guaranteed to be linearly mapped:
    for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i += 0x1000) {
        pgdir_map_page((uint32_t *)src->arch.cr3, (void *)(dest->arch.kernel_stack - PROC_KERN_STACK_SIZE + i), (void *)(dest->arch.kernel_stack - PROC_KERN_STACK_SIZE + i), 0x03);
    }

    memcpy((void *)(dest->arch.kernel_stack - PROC_KERN_STACK_SIZE), (void *)(src->arch.kernel_stack - PROC_KERN_STACK_SIZE), PROC_KERN_STACK_SIZE);

    return 0;
}