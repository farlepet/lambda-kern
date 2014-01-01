#ifndef MEM_H
#define MEM_H

#include "paging.h"

#define FIRST_PAGEDIR (((u32)&kern_end & 0xFFFFF000) + 0x1000)

#define PAGETBL_0     (FIRST_PAGEDIR + 0x1000)
#define PAGETBL_1     (PAGETBL_0 + 0x1000)
#define PAGETBL_2     (PAGETBL_1 + 0x1000)
#define PAGETBL_3     (PAGETBL_2 + 0x1000)

#define FRAMES_TABLE  (PAGETBL_3 + 0x1000)

#define FRAMES_TABLE_SIZE 0x10000

#define FRAMES_START  (FRAMES_TABLE + FRAMES_TABLE_SIZE + 0x10000)

#endif