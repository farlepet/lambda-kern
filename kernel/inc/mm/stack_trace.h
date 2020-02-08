#ifndef MEM_STACK_TRACE_H
#define MEM_STACK_TRACE_H

#include <mm/symbols.h>
#include <types.h>

/**
 * @brief Print stack trace
 * 
 * TODO: Add functionality to specify where to output stack trace
 * 
 * @param max_frames Maximum number of function calls to traverse
 * @param ebp Last saved stack pointer from which to start
 * @param saved_eip Address of function from which stack trace was triggered (optional)
 * @param symbols Symbol table
 */
void stack_trace(uint32_t max_frames, uint32_t *ebp, uint32_t saved_eip, symbol_t *symbols);

/**
 * @brief Generate stack trace from current address and stack
 * 
 * @param max_frames Maximum number of function calls to traverse
 * 
 * @see stack_trace
 */
void stack_trace_here(int max_frames);

#endif
