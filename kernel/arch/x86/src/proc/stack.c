#include <arch/proc/stack.h>
#include <arch/mm/paging.h>
#include <arch/proc/user.h>

#include <err/error.h>
#include <string.h>

int proc_copy_stack(struct kproc *dest, const struct kproc *src) {
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

int proc_copy_kernel_stack(struct kproc *dest, const struct kproc *src) {
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