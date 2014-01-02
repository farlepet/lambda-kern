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

#ifdef DEBUGGER
/**
 * Kernel debugger task
 */
__noreturn static void kbug_task()
{
	ktask_pids[KBUG_TASK_SLOT] = current_pid;

	for(;;)
	{
		struct kbug_type_msg ktm;
		recv_message(&ktm, sizeof(struct kbug_type_msg));

		switch(ktm.type)
		{
			case KBUG_PROCINFO: {	struct kbug_proc_msg kpm;
									recv_message(&kpm, sizeof(struct kbug_proc_msg));

									switch(kpm.type)
									{
										case KBUG_PROC_NPROCS: {	int i = 0;
																	int n = 0;
																	for(; i < MAX_PROCESSES; i++)
																		if(procs[i].type & TYPE_VALID) n++;

																	send_message(ktm.pid, &n, sizeof(int));
															   } break;

										case KBUG_PROC_PROCPID: {	int pid = procs[kpm.info].pid;

																	send_message(ktm.pid, &pid, sizeof(int));
																} break;
									}
								} break;

			case KBUG_CPUINFO: {	// To be implemented
							   } break;
		}
	}
}
#endif // Debugger

void init_ktasks()
{
	kerror(ERR_BOOTINFO, "Starting kernel tasks");

	kerror(ERR_BOOTINFO, "Starting kernel idle task");
	add_kernel_task(&idle_task, "_idle_", 0x100);

	kerror(ERR_BOOTINFO, "Starting kernel video task");
	add_kernel_task(&kvid_task, "kvid", 0x1000);

#ifdef DEBUGGER
	kerror(ERR_BOOTINFO, "Starting kernel debug task");
	add_kernel_task(&kbug_task, "kbug", 0x2000);
#endif // DEBUGGER
}