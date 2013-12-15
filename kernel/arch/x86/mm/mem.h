#ifndef MEM_H
#define MEM_H

#include "paging.h"

#define FIRST_PAGEDIR (((u32)&kern_end & 0xFFFFF000) + 0x1000)
#define FIRST_PAGETBL (FIRST_PAGEDIR + 0x1000)

#define APIC_PHYS 0xFEE00000
#define APIC_VIRT (FIRST_PAGETBL + 0x1000)

#define FRAMES_TABLE (APIC_VIRT + 0x1000)
#define FRAMES_START (FRAMES_TABLE + 0x1000)

#endif