#include <arch/intr/int.h>

#include <proc/ktasks.h>
#include <proc/thread.h>
#include <main/main.h>
#include <err/error.h>
#include <err/panic.h>
#include <time/time.h>
#include <io/output.h>
#include <string.h>

int ktask_pids[KTASK_SLOTS] = { 0 }; //!< PID's of these tasks

void init_ktasks() {
    if(!strlen((const char *)boot_options.init_executable)) {
        kerror(ERR_INFO, "Starting kernel terminal thread");
        if((ktask_pids[KTERM_TASK_SLOT] = thread_spawn((uintptr_t)kterm_task, NULL, "kterm", 0x1000, PRIO_DRIVER)) < 0) {
            kpanic("init_ktask: Could not spawn kernel terminal task");
        }
    }

#ifdef CONFIG_KTASK_DEBUGGER
    kerror(ERR_INFO, "Starting kernel debug thread");
    if((ktask_pids[KBUG_TASK_SLOT] = thread_spawn((uintptr_t)kbug_task, NULL, "kbug", 0x1000, PRIO_DRIVER)) < 0) {
        kpanic("init_ktask: Could not spawn kernel debug task");
    }
#endif // DEBUGGER

    kerror(ERR_INFO, "Starting kernel input thread");
    if((ktask_pids[KINPUT_TASK_SLOT] = thread_spawn((uintptr_t)kinput_task, NULL, "kinput", 0x1000, PRIO_DRIVER)) < 0) {
        kpanic("init_ktask: Could not spawn kernel input task");
    }
}
