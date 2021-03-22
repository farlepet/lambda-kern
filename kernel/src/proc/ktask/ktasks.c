#include <arch/intr/int.h>

#include <proc/ktasks.h>
#include <proc/thread.h>
#include <err/error.h>
#include <err/panic.h>
#include <time/time.h>
#include <proc/ipc.h>
#include <video.h>
#include <string.h>

int ktask_pids[KTASK_SLOTS] = { 0 }; //!< PID's of these tasks

/**
 * Get a kernel task PID
 *
 * @param n kernel task number
 * @param time max number of clock ticks to wait (0 = infinite)
 */
int get_ktask(int n, uint64_t time) {
	if(n >= KTASK_SLOTS) {
		kerror(ERR_MEDERR, "Kernel requested pid of invalid ktask: %d", n);
		return 0;
	}
	
	uint64_t end = kerneltime + time;
	while((kerneltime < end) || (time == 0)) {
		if(ktask_pids[n]) return ktask_pids[n];
		busy_wait();
	}
	kerror(ERR_MEDERR, "Could not get pid of ktask %d within %d ticks", n, time);
	return 0;
}

void init_ktasks() {
	// Small stack size here was effecting interrupts?
	kerror(ERR_BOOTINFO, "Starting kernel idle thread");
	if((ktask_pids[IDLE_TASK_SLOT] = kthread_create((void *)&idle_task, NULL, "_idle_", 0x1000, PRIO_IDLE)) < 0) {
		kpanic("init_ktask: Could not create kernel idle task");
	}

	kerror(ERR_BOOTINFO, "Starting kernel video thread");
	if((ktask_pids[KVID_TASK_SLOT] = kthread_create((void *)&kvid_task, NULL, "kvid", 0x1000, PRIO_DRIVER)) < 0) {
		kpanic("init_ktask: Could not create kernel video task");
	}

	if(!strlen((const char *)boot_options.init_executable)) {
		kerror(ERR_BOOTINFO, "Starting kernel terminal thread");
		if((ktask_pids[KTERM_TASK_SLOT] = kthread_create((void *)&kterm_task, NULL, "kterm", 0x1000, PRIO_DRIVER)) < 0) {
			kpanic("init_ktask: Could not create kernel terminal task");
		}
	}

#ifdef DEBUGGER
	kerror(ERR_BOOTINFO, "Starting kernel debug thread");
	if((ktask_pids[KVID_TASK_SLOT] = kthread_create((void *)&kbug_task, NULL, "kbug", 0x1000, PRIO_DRIVER)) < 0) {
		kpanic("init_ktask: Could not create kernel debug task");
	}
#endif // DEBUGGER

	kerror(ERR_BOOTINFO, "Starting kernel input thread");
	if((ktask_pids[KINPUT_TASK_SLOT] = kthread_create((void *)&kinput_task, NULL, "kinput", 0x1000, PRIO_DRIVER)) < 0) {
		kpanic("init_ktask: Could not create kernel input task");
	}
}
