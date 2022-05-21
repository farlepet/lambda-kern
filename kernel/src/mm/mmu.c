#include <types.h>

#include <mm/mmu.h>

int mmu_map(uintptr_t virt, uintptr_t phys, size_t size, uint32_t flags) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if (table == NULL) {
        return -1;
    }
#endif

    return mmu_map_table(table, virt, phys, size, flags);
}

int mmu_unmap(uintptr_t virt, size_t size) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if (table == NULL) {
        return -1;
    }
#endif

    return mmu_unmap_table(table, virt, size);
}

int mmu_map_get(uintptr_t virt, uintptr_t *phys) {
    mmu_table_t *table = mmu_get_current_table();
#if CHECK_STRICTNESS(LAMBDA_STRICTNESS_LOWIMPACT)
    if (table == NULL) {
        return -1;
    }
#endif

    return mmu_map_get_table(table, virt, phys);
}
