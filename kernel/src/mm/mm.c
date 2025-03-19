#include <errno.h>

#include <mm/mm.h>
#include <io/output.h>
#include <err/panic.h>
#include <err/error.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <mm/mmu.h>

extern int __kernel_text_begin,   __kernel_text_end;
extern int __kernel_rodata_begin, __kernel_rodata_end;
extern int __kernel_data_begin,   __kernel_data_end;
extern int __kernel_bss_begin,    __kernel_bss_end;

int mm_init_kernel_map(void) {
    /* @note This assumes that all sections are page-aligned */
    mmu_map((uintptr_t)&__kernel_text_begin, (uintptr_t)&__kernel_text_begin - KERNEL_OFFSET,
            (size_t)&__kernel_text_end - (size_t)&__kernel_text_begin,
            MMU_FLAG_READ | MMU_FLAG_EXEC | MMU_FLAG_KERNEL);
    mmu_map((uintptr_t)&__kernel_rodata_begin, (uintptr_t)&__kernel_rodata_begin - KERNEL_OFFSET,
            (size_t)&__kernel_rodata_end - (size_t)&__kernel_rodata_begin,
            MMU_FLAG_READ | MMU_FLAG_KERNEL);
    mmu_map((uintptr_t)&__kernel_data_begin, (uintptr_t)&__kernel_data_begin - KERNEL_OFFSET,
            (size_t)&__kernel_data_end - (size_t)&__kernel_data_begin,
            MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL);
    mmu_map((uintptr_t)&__kernel_bss_begin, (uintptr_t)&__kernel_bss_begin - KERNEL_OFFSET,
            (size_t)&__kernel_bss_end - (size_t)&__kernel_bss_begin,
            MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_KERNEL);

    return 0;
}

int mm_check_addr(const void *addr) {
    int flags = mmu_map_get((uintptr_t)addr, NULL);
    if(flags < 0) {
        return 0;
    }

    return (flags & MMU_FLAG_WRITE) ? 2 :
           (flags & MMU_FLAG_READ)  ? 1 : 0;
}

void *mm_translate_proc_addr(struct kproc *proc, const void *addr) {
    uintptr_t phys = 0;
    if(mmu_map_get_table(proc->mmu_table, (uintptr_t)addr, &phys) < 0) {
        return NULL;
    }

    return (void *)phys;
}

int mm_proc_mmap_add(struct kproc *proc, uintptr_t phys, uintptr_t virt, size_t sz) {
    kdebug(DEBUGSRC_MM, ERR_DEBUG, "mm_proc_mmap_add(%s, %08X, %08X, %d)", proc->name, phys, virt, sz);
    
    struct kproc_mem_map_ent *mmap_ent = (struct kproc_mem_map_ent *)kmalloc(sizeof(struct kproc_mem_map_ent));
    if(!mmap_ent) {
        return -ENOMEM;
    }

    mmap_ent->virt_address = virt;
    mmap_ent->phys_address = phys;
    mmap_ent->length       = sz;
    mmap_ent->next         = NULL;

    struct kproc_mem_map_ent **last = &proc->mmap;
    while(*last) {
        last = &(*last)->next;
    }
    *last = mmap_ent;

    return 0;
}

int mm_proc_mmap_remove_virt(struct kproc *proc, uintptr_t virt) {
    kdebug(DEBUGSRC_MM, ERR_DEBUG, "mm_proc_mmap_remove_virt(%s, %08X)", proc->name, virt);
    
    struct kproc_mem_map_ent *mmap_ent  = proc->mmap;
    struct kproc_mem_map_ent *mmap_prev = NULL;
    
    while(mmap_ent) {
        if(mmap_ent->virt_address == virt) {
            if(mmap_prev) {
                mmap_prev->next = mmap_ent->next;
            } else {
                proc->mmap = mmap_ent->next;
            }
            kfree(mmap_ent);
            return 0;
        }
        mmap_prev = mmap_ent;
        mmap_ent  = mmap_ent->next;
    }

    return -EINVAL;
}

int mm_proc_mmap_remove_phys(struct kproc *proc, uintptr_t phys) {
    kdebug(DEBUGSRC_MM, ERR_DEBUG, "mm_proc_mmap_remove_phys(%s, %08X)", proc->name, phys);
    
    struct kproc_mem_map_ent *mmap_ent  = proc->mmap;
    struct kproc_mem_map_ent *mmap_prev = NULL;
    
    while(mmap_ent) {
        if(mmap_ent->phys_address == phys) {
            if(mmap_prev) {
                mmap_prev->next = mmap_ent->next;
            } else {
                proc->mmap = mmap_ent->next;
            }
            kfree(mmap_ent);
            return 0;
        }
        mmap_prev = mmap_ent;
        mmap_ent  = mmap_ent->next;
    }

    return -EINVAL;
}
