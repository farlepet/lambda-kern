#include <arch/mm/alloc.h>

#include <proc/ktasks.h>
#include <time/time.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <mm/mm.h>
#include <video.h>

/**
 * Handles video I/O
 */
__noreturn void kvid_task()
{
	ktask_pids[KVID_TASK_SLOT] = current_pid;

	for(;;)
	{
		int ret;
		struct ipc_message_user umsg;

		while((ret = ipc_user_recv_message_blocking(&umsg)) < 0)
		{
			kerror(ERR_MEDERR, "KVID: IPC error: %d", ret);
		}
		
		void *data = kmalloc(umsg.length);

		ipc_user_copy_message(umsg.message_id, data);

		switch(((struct kvid_type_msg *)data)->type)
		{
			case KVID_PRINT: {	struct kvid_print_m *m = (struct kvid_print_m *)data;
								if(mm_check_addr(m->kpm.string)) {
									kprintf("%s", m->kpm.string);
								} else {
									kerror(ERR_SMERR, "kvid: Given non-existant data pointer (%08X) from PID %d!", m->kpm.string, umsg.src_pid);
								}
							 } break;
			
			case KVID_KERROR: {	struct kvid_kerror_m *m = (struct kvid_kerror_m *)data;
								if(mm_check_addr(m->kkm.string)) {
									kerror(m->kkm.error_level, "%s", m->kkm.string); // Maybe we should check the string and PID or something?
								} else {
									kerror(ERR_SMERR, "kvid: Given non-existant data pointer (%08X) from PID %d!", m->kkm.string, umsg.src_pid);
								}
							  } break;
		}

		kfree(data);
	}
}