#ifndef MEM_H
#define MEM_H

#include "paging.h"

#define FRAMES_START  (((u32)&kern_end & 0xFFFFF000) + 0x1000) //!< Start of page frames

#endif