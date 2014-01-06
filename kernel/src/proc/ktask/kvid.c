#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <video.h>

/**
 * Handles video I/O
 */
__noreturn void kvid_task()
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