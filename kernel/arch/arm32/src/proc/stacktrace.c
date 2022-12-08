#include <arch/proc/stacktrace.h>
#include <mm/mm.h>

#include <io/output.h>

static int _next_frame(arch_stackframe_t *);
static int _print_func(const arch_stackframe_t *, const symbol_t *);
static void _print_data(uint32_t *, uint32_t);

void arch_stacktrace(void *fp, uintptr_t pc, uint32_t max_frames, const symbol_t *symbols) {
    if((fp         == NULL) ||
       (max_frames == 0)) {
        return;
    }

    if(symbols == NULL) {
        symbols = sym_functions;
    }

    kprintf("Stack Trace:\n");
    if(pc) {
        //_print_func(pc, symbols);
        /* TODO */
    }
    kprintf("  FP: %08x\n", fp);

    arch_stackframe_t frame;
    frame.fp = (uintptr_t)fp;
    if(_next_frame(&frame)) {
        return;
    }

    for(uint32_t i = 0; i < max_frames; i++) {
        /* TODO: Check if frame is valid */
        kprintf("  SP: %08x, FP: %08x\n", frame.sp, frame.fp);
        if(!frame.fp) {
            break;
        }
        _print_func(&frame, symbols);

        if(_next_frame(&frame)) {
            break;
        }
    }
}

void stacktrace_here(int max_frames) {
    arch_stacktrace(__builtin_frame_address(0), (uint32_t)&stacktrace_here, max_frames, NULL);
}

static int _next_frame(arch_stackframe_t *frame) {
    if(frame == NULL) return 1;

    uintptr_t fp = frame->fp;

    if(!mm_check_addr((void *)(fp - 12)) ||
       !mm_check_addr((void *)(fp - 4))) {
        return 1;
    }

    frame->fp = *(uint32_t *)(fp - 12);
    frame->sp = *(uint32_t *)(fp - 8);
    frame->pc = *(uint32_t *)(fp - 4);

    return 0;
}

static int _pc_valid(uintptr_t pc) {
    /* NOTE: Making this an inline function, as there will likely be more logic
     * here in the future */
    if(pc < (uintptr_t)&kern_start) return 0;
    /* NOTE: Temporary, as we do not currently allow program loading */
    if(pc > (uintptr_t)&kern_end) return 0;

    return 1;
}

static int _frame_valid(const arch_stackframe_t *frame) {
    /* @todo Make this check a little cleaner, or just incorporate directly into
     * _print_func() */
    return (mm_check_addr(frame)                            &&
            mm_check_addr((void *)frame->fp)                &&
            mm_check_addr(*(uint32_t **)frame->fp)          &&
            mm_check_addr(*(*(uint32_t ***)frame->fp - 16)) &&
            _pc_valid(frame->pc));
}

static int _print_func(const arch_stackframe_t *frame, const symbol_t *symbols __unused) {
    if(!_frame_valid(frame)) {
        return 1;
    }

    uint32_t _pc = *(uint32_t *)frame->fp;
    /* NOTE: GCC documentation states pc - 12 */
    uint32_t inf = *(uint32_t *)(_pc - 16);
    if((inf & 0xFF000000) == 0xFF000000) {
        char *name = (char *)(_pc - 16 - (inf & 0x00FFFFFF));
        uint32_t _func = _pc - 12;
        uint32_t _off  = frame->pc - _func;
        kprintf("  [%08x] <0x%08x +%03d> %s\n", frame->pc, _func, _off, name);
    } else {
        kprintf("  [%08x]\n", frame->pc);
    }

    return 0;
}

__unused
static void _print_data(uint32_t *sp, uint32_t len) {
    len >>= 2;
    kprintf("  -> ");
    uint32_t i = 0;
    for(; i < len; i++) {
        if(i != 0 && (i % 4) == 0) {
            kprintf("\n     ");
        }
        /* TODO: Check if address is valid */
        kprintf("%08x ", sp[-(i+1)]);
    }
    kprintf("\n");
}
