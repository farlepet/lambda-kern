#include <proc/ktasks.h>
#include <time/time.h>
#include <err/error.h>
#include <mm/alloc.h>
#include <io/output.h>

#ifdef CONFIG_KTASK_DEBUGGER

static void idebug();

/**
 * Kernel debugger task
 * 
 * @todo Update to use something other than IPC, or remove altogether (likely
 * the latter).
 */
__noreturn void kbug_task() {
    for(;;) {
        switch(((struct kbug_type_msg *)data)->type) {
            case KBUG_PROCINFO: {
                //struct kbug_type_proc_msg *ktpm = (struct kbug_type_proc_msg *)data;

                /* TODO: Update to new process storage structure.
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
                        struct kproc *_proc = proc_by_pid(ktpm->kpm.pid);
                        kproc_to_uproc(_proc, &proc);

                        ipc_user_create_and_send_message(umsg.src_pid, &proc, sizeof(struct uproc));
                    } break;
                }
                */
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
    
#ifdef CONFIG_LOG_COLORCODE
        kprintf("\e[41mPID  UID  GID      SENT  RECEIVED BLCK PRI      TYPE SCHED_C LAST_EIP  SYSCALLS NAME            \e[0m\n");
#else
        kprintf("PID  UID  GID      SENT  RECEIVED BLCK PRI      TYPE SCHED_C  SYSCALLS NAME\n");
#endif

    int i = 0;
    for(; i < MAX_PROCESSES; i++)
        /* TODO */
        /*if(procs[i].type & TYPE_VALID) {
            kprintf("% 02d % 02d % 02d %8d %8d   %c  %02d %08X %8d %8d %s\n", 
                procs[i].pid, procs[i].uid, procs[i].gid,
                procs[i].book.sent_msgs, procs[i].book.recvd_msgs,
                ((procs[i].blocked != 0) ? 'Y' : 'N'),
                procs[i].prio, procs[i].type, procs[i].book.schedule_count,
                procs[i].book.syscall_count, procs[i].name
            );
        }*/

    kerror(ERR_INFO, "IDEBUG finished");
}

#endif // Debugger
