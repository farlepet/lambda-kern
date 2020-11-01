#ifndef MEM_H
#define MEM_H

#include <arch/mm/paging.h>

#define FRAMES_START  (((uint32_t)&kern_end & 0xFFFFF000) + 0x1000) //!< Start of page frames

#endif