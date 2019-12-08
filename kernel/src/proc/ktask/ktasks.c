#include <proc/ktasks.h>
#include <err/error.h>
#include <time/time.h>
#include <proc/ipc.h>
#include <intr/int.h>
#include <video.h>

int ktask_pids[KTASK_SLOTS]; //!< PID's of these tasks

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
	kerror(ERR_BOOTINFO, "Starting kernel idle task");
	add_kernel_task((void *)&idle_task, "_idle_", 0x100, PRIO_IDLE);

	kerror(ERR_BOOTINFO, "Starting kernel video task");
	add_kernel_task((void *)&kvid_task, "kvid", 0x1000, PRIO_DRIVER);

#ifdef DEBUGGER
	kerror(ERR_BOOTINFO, "Starting kernel debug task");
	add_kernel_task((void *)&kbug_task, "kbug", 0x2000, PRIO_KERNEL);
#endif // DEBUGGER

	kerror(ERR_BOOTINFO, "Starting kernel input task");
	add_kernel_task((void *)&kinput_task, "kinput", 0x1000, PRIO_DRIVER);

	kerror(ERR_BOOTINFO, "Starting kernel RNG task");
	add_kernel_task((void *)&krng_task, "krng", 0x1000, PRIO_DRIVER);

	kerror(ERR_BOOTINFO, "Starting kernel terminal task");
	add_kernel_task((void *)&kterm_task, "kterm", 0x2000, PRIO_DRIVER);
}
