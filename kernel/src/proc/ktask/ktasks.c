#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <intr/int.h>
#include <video.h>

int ktask_pids[KTASK_SLOTS]; //!< PID's of these tasks

void init_ktasks()
{
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
}