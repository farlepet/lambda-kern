#ifndef PROC_STACKTRACE_H
#define PROC_STACKTRACE_H

#include <mm/symbols.h>
#include <types.h>

typedef struct {
    uintptr_t fp; /*!< Frame Pointer */
    uintptr_t sp; /*!< Stack Pointer */
    uintptr_t pc; /*!< Program Counter / Return Address */
} arch_stackframe_t;

/**
 * @brief Print a stacktrace
 * 
 * @param fp Frame pointer 
 * @param pc Last program counter, if available
 * @param max_frames Maximum number of frames to print
 * @param symbols Pointer to extra symbols, if applicable
 */
void arch_stacktrace(void *fp, uintptr_t pc, uint32_t max_frames, const symbol_t *symbols);

/**
 * @brief 
 * 
 * @param max_frames 
 */
void stacktrace_here(int max_frames);

#endif
