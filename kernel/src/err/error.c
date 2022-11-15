#include <lambda/export.h>
#include <time/time.h>
#include <err/error.h>
#include <config.h>
#include <types.h>
#include <io/output.h>
#include <math.h>

static uint8_t _err_level[DEBUGSRC_MAX] = {
    [0 ... (DEBUGSRC_MAX-1)] = ERR_INFO
};

static char *debug_names[DEBUGSRC_MAX] = {
    [DEBUGSRC_MISC]    = "MISC",
    [DEBUGSRC_FS]      = "  FS",
    [DEBUGSRC_MM]      = "  MM",
    [DEBUGSRC_PROC]    = "PROC",
    [DEBUGSRC_EXEC]    = "EXEC",
    [DEBUGSRC_SYSCALL] = " SCL",
    [DEBUGSRC_MODULE]  = " MOD",
};

void kdebug(debug_source_e src, error_level_e lvl, const char *msg, ...) {
    if(SAFETY_CHECK(src >= DEBUGSRC_MAX) ||
       (lvl < _err_level[src])) {
        return;
    }

    /* NOTE: This is not very effecient. */
    uint64_t sec;
    uint64_t nano;
    sec = udiv64(kerneltime, 1000000000, &nano);

    /* TODO: Write to a buffer, rather than directly writing to the kernel's
     * output device. */

#if (KERNEL_COLORCODE)
    kprintf("\e[31m[\e[32m%05llu.%03u\e[31m] [\e[33m%s\e[31m]\e[0m ",
    sec, (uint32_t)nano / 1000000, debug_names[src], &msg);
#else
    kprintf("[%05llu.%03u] [%s] ",
    sec, (uint32_t)nano / 1000000, debug_names[src], &msg);
#endif

    __builtin_va_list varg;
    __builtin_va_start(varg, msg);
    kprintv(msg, varg);
    __builtin_va_end(varg);
    kput('\n');
}
EXPORT_FUNC(kdebug);

void kdebug_set_errlvl(debug_source_e src, error_level_e lvl) {
    if(src >= DEBUGSRC_MAX) {
        return;
    }

    _err_level[src] = lvl;
}

error_level_e kdebug_get_errlvl(debug_source_e src) {
    if(src >= DEBUGSRC_MAX) {
        return ERR_NONE;
    }

    return _err_level[src];
}
