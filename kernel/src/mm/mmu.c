#include <string.h>

#include <types.h>
#include <mm/mmu.h>

int mmu_map(uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if(table == NULL) {
        return -1;
    }
#endif

    return mmu_map_table(table, virt, phys, size, flags);
}

int mmu_unmap(uintptr_t virt, size_t size) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if(table == NULL) {
        return -1;
    }
#endif

    return mmu_unmap_table(table, virt, size);
}

int mmu_map_get(uintptr_t virt, uintptr_t *phys) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if(table == NULL) {
        return -1;
    }
#endif

    return mmu_map_get_table(table, virt, phys);
}

int mmu_map_save(mmu_map_entry_t *entry, uintptr_t virt) {
    int flags;
    if((flags = mmu_map_get(virt, &entry->phys)) < 0) {
        return -1;
    }
    entry->flags = (uint32_t)flags;
    entry->virt  = virt;

    return 0;
}

int mmu_map_restore(mmu_map_entry_t *entry) {
    return mmu_map(entry->virt, entry->phys, mmu_get_pagesize(), entry->flags);
}

int mmu_copy_data(mmu_table_t *dmmu, uintptr_t dvirt, mmu_table_t *smmu, uintptr_t svirt, size_t size) {
    size_t off      = 0;
    size_t pagesz   = mmu_get_pagesize();
    size_t sizemask = pagesz - 1; /* Assumes page size is power of 2 */

#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if((dvirt & ~sizemask) != (svirt & ~sizemask)) {
        return -1;
    }
#endif

    while(off < size) {
        int sflag, dflag;
        uintptr_t sphys = 0, dphys = 0;
        mmu_map_entry_t sent, dent;

        size_t    to_copy = (size - off) & sizemask;
        uintptr_t src     = svirt + off;
        uintptr_t dest    = dvirt + off;
        if((to_copy + (src & sizemask)) > pagesz) {
           to_copy = pagesz - (src & sizemask);
        }

        if((sflag = mmu_map_get_table(smmu, src, &sphys) < 0)  ||
           !(sflag & MMU_FLAG_READ)                            ||
           (dflag = mmu_map_get_table(dmmu, dest, &dphys) < 0) ||
           !(sflag & MMU_FLAG_WRITE)) {
            return -1;
        }

        off += to_copy;

        if(sphys == dphys) {
            /* Data is shared */
            continue;
        }

        if(src & sizemask) {
            sphys += (src & sizemask);
            dphys += (src & sizemask);
        }

        int src_saved  = (mmu_map_save(&sent, src)  == 0);
        int dest_saved = (mmu_map_save(&dent, dest) == 0);

        /* Map of size 1 in order to only map one page without masking */
        mmu_map(sphys, sphys, 1, MMU_FLAG_READ);
        mmu_map(dphys, dphys, 1, MMU_FLAG_WRITE);

        memcpy((void *)dphys, (void *)sphys, to_copy);

        if(src_saved)  { mmu_map_restore(&sent); }
        else           { mmu_unmap(sphys, 1); }
        if(dest_saved) { mmu_map_restore(&dent); }
        else           { mmu_unmap(dphys, 1); }
    }

    return 0;
}
