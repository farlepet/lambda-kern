#include <mm/alloc.h>
#include <mm/mmap.h>
#include <mm/mmu.h>
#include <mm/mm.h>
#include <proc/mtask.h>

uintptr_t mmap(uintptr_t addr, size_t len, uint32_t prot, uint32_t flags, int filedes, off_t off) {
    if(!(flags & SYSCALL_MMAPFLAG_ANONYMOUS)) {
        /* @todo File mappings not currently supported */
        return 0;
    }

    size_t pagesz = mmu_get_pagesize();

    if(len % pagesz) {
        len = (len & ~(pagesz - 1)) + pagesz;
    }

    if(off % pagesz) {
        return -1;
    }

    uintptr_t phys = (uintptr_t)kamalloc(len, pagesz);
    uintptr_t virt;

    if(flags & SYSCALL_MMAPFLAG_FIXED) {
        if((uintptr_t)addr % pagesz) {
            return 0;
        }
        virt = addr;
    } else {
        virt = phys;
    }

    uint32_t mmu_flags = ((prot & SYSCALL_MMAPPROT_READ)  ? MMU_FLAG_READ  : 0) |
                         ((prot & SYSCALL_MMAPPROT_WRITE) ? MMU_FLAG_WRITE : 0) |
                         ((prot & SYSCALL_MMAPPROT_EXEC)  ? MMU_FLAG_EXEC  : 0);

    /* TODO: Check that virtual memory region is free */
    kproc_t *proc = mtask_get_curr_process();
    mmu_map_table(proc->mmu_table, virt, phys, len, mmu_flags);

    mm_proc_mmap_add(proc, phys, virt, len);

    /* TODO */
    (void)filedes;

    return virt;
}

int munmap(uintptr_t addr, size_t len) {
    /* TODO */
    (void)addr;
    (void)len;

    return 0;
}
