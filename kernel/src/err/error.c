#include <lambda/export.h>
#include <proc/atomic.h>
#include <intr/intr.h>
#include <time/time.h>
#include <err/error.h>
#include <config.h>
#include <types.h>
#include <video.h>

static uint8_t _err_level[DEBUGSRC_MAX] = {
    [0 ... (DEBUGSRC_MAX-1)] = ERR_INFO
};

static lock_t kerror_lock = 0; //!< Only 1 message can be printed at a time

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

    if(interrupts_enabled()) lock_for(&kerror_lock, 100); // We don't want something like a kernel message from a lost task stopping us

#if (KERNEL_COLORCODE)
    kprintf("\e[31m[\e[32m%X%08X\e[31m] [\e[33m%s\e[31m]\e[0m ", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime, debug_names[src]);
#else
    kprintf("[%X%08X] [%s]", (uint32_t)(kerneltime >> 32), (uint32_t)kerneltime, debug_names[src]);
#endif

    __builtin_va_list varg;
    __builtin_va_start(varg, msg);
    kprintv(msg, varg);
    __builtin_va_end(varg);
    kput('\n');
    
    if(interrupts_enabled()) unlock(&kerror_lock);
}
EXPORT_FUNC(kdebug);
