#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <video.h>

#ifdef DEBUGGER

static void idebug();

/**
 * Kernel debugger task
 */
__noreturn void kbug_task()
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

										case KBUG_PROC_UPROC: {	struct uproc proc;
																int idx = proc_by_pid(kpm.pid);
																kproc_to_uproc(&procs[idx], &proc);

																send_message(ktm.pid, &proc, sizeof(struct uproc));
															  } break;
									}
								} break;

			case KBUG_CPUINFO: {	// To be implemented
							   } break;

			case KBUG_MEMINFO: {	// TODO: Check that the requesting process is root or a kernel process
									struct kbug_mem_msg kmm;
									recv_message(&kmm, sizeof(struct kbug_mem_msg));

									// TODO: Check that this won't cause a page fault
									send_message(ktm.pid, (void *)kmm.mem_addr, (int)kmm.mem_len);
							   } break;

			case KBUG_IDEBUG: {	idebug();
							  } break;
		}
	}
}

static void idebug()
{
	kerror(ERR_INFO, "IDEBUG started");
	
	kprintf("\e[41mPID UID GID SENT     RECEIVED\e[0m\n");
	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].type & TYPE_VALID)
		{
			kprintf("% 02d % 02d % 02d %8d %8d\n", procs[i].pid, procs[i].uid, procs[i].gid, procs[i].book.sent_msgs, procs[i].book.recvd_msgs);
		}

	kerror(ERR_INFO, "IDEBUG finished");
}

#endif // Debugger