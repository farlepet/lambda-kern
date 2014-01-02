#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <intr/int.h>
#include <video.h>

int ktask_pids[KTASK_SLOTS]; //!< PID's of these tasks


/**
 * A basic task that only loops, 
 * ran if no other tasks are available
 */
__noreturn static void idle_task()
{
	ktask_pids[IDLE_TASK_SLOT] = current_pid;

	for(;;) busy_wait();
}

/**
 * Handles VGA I/O
 */
__noreturn static void kvid_task()
{
	ktask_pids[KVID_TASK_SLOT] = current_pid;

	for(;;)
	{
		struct kvid_type_msg ktm;
		recv_message(&ktm, sizeof(struct kvid_type_msg));

		switch(ktm.type)
		{
			case KVID_PRINT: {	struct kvid_print_msg kpm;
								recv_message(&kpm, sizeof(struct kvid_print_msg));
								kprintf("%s", kpm.string);
							 } break;

			case KVID_KERROR: {	struct kvid_kerror_msg kkm;
								recv_message(&kkm, sizeof(struct kvid_kerror_msg));
								kerror(kkm.error_level, "%s", kkm.string); // Maybe we should check the string and PID or something?
							  } break;
		}
	}
}




void init_ktasks()
{
	kerror(ERR_BOOTINFO, "Starting kernel tasks");

	kerror(ERR_BOOTINFO, "Starting kernel idle task");
	add_kernel_task(&idle_task, "Kernel idle task", 0x100);

	kerror(ERR_BOOTINFO, "Starting kernel video task");
	add_kernel_task(&kvid_task, "Kernel video task", 0);
}