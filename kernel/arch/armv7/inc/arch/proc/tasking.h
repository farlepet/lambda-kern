#ifndef ARCH_ARMV7_PROC_TASKING_H
#define ARCH_ARMV7_PROC_TASKING_H

#include <stdint.h>

typedef struct {
    struct {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r4;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t sp;
        uint32_t lr;
        uint32_t cpsr;
        uint32_t pc;
    } regs;                     //!< Saved registers

	uint32_t kernel_stack;      //!< Kernel stack
	uint32_t kernel_stack_size; //!< Size of kernel stack

	uint32_t stack_beg;         //!< Beginning of stack
	uint32_t stack_end;         //!< Current end of stack
} kproc_arch_t;

#endif