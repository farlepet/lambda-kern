#include <arch/intr/int.h>

#include <proc/ktasks.h>
#include <err/error.h>
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
	kerror(ERR_BOOTINFO, "Starting kernel idle task");
	ktask_pids[IDLE_TASK_SLOT] = add_kernel_task((void *)&idle_task, "_idle_", 0x1000, PRIO_IDLE);

	kerror(ERR_BOOTINFO, "Starting kernel video task");
	ktask_pids[KVID_TASK_SLOT] = add_kernel_task((void *)&kvid_task, "kvid", 0x1000, PRIO_DRIVER);

	if(!strlen((const char *)boot_options.init_executable)) {
		kerror(ERR_BOOTINFO, "Starting kernel terminal task");
		ktask_pids[KTERM_TASK_SLOT] = add_kernel_task((void *)&kterm_task, "kterm", 0x2000, PRIO_DRIVER);
	}

#ifdef DEBUGGER
	kerror(ERR_BOOTINFO, "Starting kernel debug task");
	ktask_pids[KBUG_TASK_SLOT] = add_kernel_task((void *)&kbug_task, "kbug", 0x2000, PRIO_KERNEL);
#endif // DEBUGGER

	kerror(ERR_BOOTINFO, "Starting kernel input task");
	ktask_pids[KINPUT_TASK_SLOT] = add_kernel_task((void *)&kinput_task, "kinput", 0x1000, PRIO_DRIVER);
}
