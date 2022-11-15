#include <arch/proc/stack_trace.h>
#include <mm/mm.h>

#include <io/output.h>

#define PRINT_STACK_DATA 0

#if (PRINT_STACK_DATA)
static void stack_trace_print_data(uint32_t *ebp, uint32_t len);
#endif

static int stack_trace_print_func(uint32_t addr, symbol_t *symbols);
static int find_function(uint32_t addr, symbol_t *symbols);
extern void start(void);

void stack_trace(uint32_t max_frames, uint32_t *ebp, uint32_t saved_eip, symbol_t *symbols) {
    if(ebp == NULL) {
        return;
    }

    // TODO: Check address (if in kernelm use sym_functions)
    if(symbols == NULL) symbols = sym_functions;

    kprintf("Stack Trace:\n", symbols);
    if(saved_eip) stack_trace_print_func(saved_eip, symbols);
    uint32_t fr = 0;
    for(; fr < max_frames; fr++) {
        if(!mm_check_addr(ebp) ||
           !mm_check_addr(ebp+4)) {
            kprintf("  EBP (%08x), or EBP+4, points to non-present page!\n", ebp);
            break;
        }
        uint32_t eip = ebp[1];
        if(stack_trace_print_func(eip, symbols) < 0) break;
        uint32_t *oebp = ebp;
        ebp = (uint32_t *)ebp[0];
        uint32_t frame_size = (uint32_t)(ebp - oebp);
        
        if(frame_size > 32) frame_size = 32;
#if (PRINT_STACK_DATA)
        stack_trace_print_data(ebp, frame_size);
#endif
    }
}

static int stack_trace_print_func(uint32_t eip, symbol_t *symbols) {
    if(eip == 0) return -1;
    if(eip < (uint32_t)&kern_start) return -2;
    
    int idx = find_function(eip, symbols);
    if(idx >= 0) {
        uint32_t addr = symbols[idx].addr;
        if(symbols == sym_functions) {
            addr += (uint32_t)&kern_start;
        }
        kprintf("  [%08x] <0x%08x +%3d> %s\n", eip, addr, eip - addr, symbols[idx].name);
        if(addr == (uint32_t)&start) {
            kprintf("  -- End of kernel stack --\n");
            return -1;
        }
    } else if(symbols == sym_functions) {
        kprintf("  [%08x] %s\n", eip, mm_check_addr((void *)eip) ? "" : "[np]");
    } else {
        return stack_trace_print_func(eip, sym_functions);
    }
    return 0;
}

#if (PRINT_STACK_DATA)
static void stack_trace_print_data(uint32_t *ebp, uint32_t len) {
    len >>= 2;
    kprintf("  -> ");
    uint32_t i = 0;
    for(; i < len; i++) {
        if(i != 0 && (i % 4) == 0) {
            kprintf("\n     ");
        }
        if(!mm_check_addr((void *)((uint32_t)ebp - (i + 1)*4))) {
            kprintf("\n");
            return;
        }
        kprintf("%08X ", ebp[-(i+1)]);
    }
    kprintf("\n");
}
#endif /* (PRINT_STACK_DATA) */

static int find_function(uint32_t addr, symbol_t *symbols) {
    if(symbols == sym_functions) {
        addr -= (uint32_t)&kern_start;
    }
    int i = 0;
    int idx = -1;
    uint32_t laddr = 0;
    symbol_t *func = NULL;
    while((func = &symbols[i])->addr != 0xFFFFFFFF) {
        if(func->addr <= addr) {
            if(func->addr > laddr) {
                if(func->size > 0) {
                    if(func->addr + func->size > addr) {
                        laddr = func->addr;
                        idx = i;
                    }
                } else {
                    laddr = func->addr;
                    idx = i;
                }
            }
        }
        i++;
    }
    return idx;
}


void stack_trace_here(int max_frames) {
    stack_trace(max_frames, __builtin_frame_address(0), (uint32_t)&stack_trace_here, NULL);
}