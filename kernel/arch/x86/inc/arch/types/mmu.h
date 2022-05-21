#ifndef ARCH_TYPES_MMU_H
#define ARCH_TYPES_MMU_H

#include <stdint.h>

#define PAGE_SZ           4096
#define PGDIR_ENTRIES     1024
#define PGTBL_ENTRIES     1024

typedef struct {
    uint32_t pagedir_ents[PGDIR_ENTRIES];
} mmu_table_t;

#endif
