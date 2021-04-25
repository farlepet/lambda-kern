#include <arch/intr/int.h>

#include <proc/ktasks.h>
#include <proc/thread.h>
#include <err/error.h>
#include <err/panic.h>
#include <time/time.h>
#include <video.h>
#include <string.h>

int ktask_pids[KTASK_SLOTS] = { 0 }; //!< PID's of these tasks

void init_ktasks() {
	// Small stack size here was effecting interrupts?
	kerror(ERR_BOOTINFO, "Starting kernel idle thread");
	if((ktask_pids[IDLE_TASK_SLOT] = kthread_create((void *)&idle_task, NULL, "_idle_", 0x1000, PRIO_IDLE)) < 0) {
		kpanic("init_ktask: Could not create kernel idle task");
	}

	if(!strlen((const char *)boot_options.init_executable)) {
		kerror(ERR_BOOTINFO, "Starting kernel terminal thread");
		if((ktask_pids[KTERM_TASK_SLOT] = kthread_create((void *)&kterm_task, NULL, "kterm", 0x1000, PRIO_DRIVER)) < 0) {
			kpanic("init_ktask: Could not create kernel terminal task");
		}
	}

#if DEBUGGER
	kerror(ERR_BOOTINFO, "Starting kernel debug thread");
	if((ktask_pids[KBUG_TASK_SLOT] = kthread_create((void *)&kbug_task, NULL, "kbug", 0x1000, PRIO_DRIVER)) < 0) {
		kpanic("init_ktask: Could not create kernel debug task");
	}
#endif // DEBUGGER

	kerror(ERR_BOOTINFO, "Starting kernel input thread");
	if((ktask_pids[KINPUT_TASK_SLOT] = kthread_create((void *)&kinput_task, NULL, "kinput", 0x1000, PRIO_DRIVER)) < 0) {
		kpanic("init_ktask: Could not create kernel input task");
	}
}
