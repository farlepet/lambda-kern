#ifndef ARCH_TYPES_MMU_H
#define ARCH_TYPES_MMU_H

#include <stdint.h>

typedef struct {
    uint32_t mmu_table[4096];
} mmu_table_t;

#endif
