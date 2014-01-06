#include <proc/ktasks.h>
#include <intr/int.h>

/**
 * A basic task that only loops, 
 * ran if no other tasks are available
 */
__noreturn void idle_task()
{
	ktask_pids[IDLE_TASK_SLOT] = current_pid;

	for(;;) busy_wait();
}