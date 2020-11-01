#include <proc/ktasks.h>
#include <time/time.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <mm/alloc.h>
#include <config.h>
#include <video.h>

#if DEBUGGER

static void idebug();

/**
 * Kernel debugger task
 */
__noreturn void kbug_task() {
	ktask_pids[KBUG_TASK_SLOT] = current_pid;

	for(;;) {
		/*struct kbug_type_msg ktm;
		recv_message(&ktm, sizeof(struct kbug_type_msg));*/

		int ret;
		struct ipc_message_user umsg;
		while((ret = ipc_user_recv_message_blocking(&umsg)) < 0) {
			kerror(ERR_MEDERR, "KBUG: IPC error: %d", ret);
		}

		void *data = kmalloc(umsg.length);

		ipc_user_copy_message(umsg.message_id, data);

		switch(((struct kbug_type_msg *)data)->type) {
			case KBUG_PROCINFO: {
				struct kbug_type_proc_msg *ktpm = (struct kbug_type_proc_msg *)data;

				switch(ktpm->kpm.type) {
					case KBUG_PROC_NPROCS: {
						int i = 0;
						int n = 0;
						for(; i < MAX_PROCESSES; i++)
							if(procs[i].type & TYPE_VALID) n++;

						ipc_user_create_and_send_message(umsg.src_pid, &n, sizeof(int));
					} break;

					case KBUG_PROC_PROCPID: {
						int pid = procs[ktpm->kpm.info].pid;

						ipc_user_create_and_send_message(umsg.src_pid, &pid, sizeof(int));
					} break;

					case KBUG_PROC_UPROC: {
						struct uproc proc;
						int idx = proc_by_pid(ktpm->kpm.pid);
						kproc_to_uproc(&procs[idx], &proc);

						ipc_user_create_and_send_message(umsg.src_pid, &proc, sizeof(struct uproc));
					} break;
				}
			} break;

			case KBUG_CPUINFO:
				// To be implemented
				break;

			case KBUG_MEMINFO: { // TODO: Check that the requesting process is root or a kernel process
				struct kbug_type_mem_msg *ktmm = (struct kbug_type_mem_msg *)data;
				//recv_message(&kmm, sizeof(struct kbug_mem_msg));

				// TODO: Check that this won't cause a page fault
				ipc_user_create_and_send_message(umsg.src_pid, (void *)(ptr_t)ktmm->kmm.mem_addr, (int)ktmm->kmm.mem_len);
			} break;

			case KBUG_IDEBUG:
				idebug();
				break;
		}

		kfree(data);
	}
}

static void idebug() {
	kerror(ERR_INFO, "IDEBUG started");
	
	if(KERNEL_COLORCODE)
		kprintf("\e[41mPID  UID  GID      SENT  RECEIVED BLCK PRI      TYPE SCHED_C LAST_EIP  SYSCALLS NAME            \e[0m\n");
	else
		kprintf("PID  UID  GID      SENT  RECEIVED BLCK PRI      TYPE SCHED_C  SYSCALLS NAME\n");

	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].type & TYPE_VALID) {
			kprintf("% 02d % 02d % 02d %8d %8d   %c  %02d %08X %8d %8d %s\n", 
				procs[i].pid, procs[i].uid, procs[i].gid,
				procs[i].book.sent_msgs, procs[i].book.recvd_msgs,
				((procs[i].blocked != 0) ? 'Y' : 'N'),
				procs[i].prio, procs[i].type, procs[i].book.schedule_count,
				procs[i].book.syscall_count, procs[i].name
			);
		}

	kerror(ERR_INFO, "IDEBUG finished");
}

#endif // Debugger
