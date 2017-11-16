#ifndef MEM_STACK_TRACE_H
#define MEM_STACK_TRACE_H

#include <mm/symbols.h>
#include <types.h>

void stack_trace(u32 max_frames, u32 *ebp, u32 saved_eip, symbol_t *symbols);

void stack_trace_here(int max_frames);

#endif
