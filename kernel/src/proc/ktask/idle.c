#include <arch/intr/int.h>

#include <proc/ktasks.h>

/**
 * A basic task that only loops, 
 * ran if no other tasks are available
 */
__noreturn void idle_task()
{
	ktask_pids[IDLE_TASK_SLOT] = curr_proc->pid;

	for(;;)
	{
		busy_wait();
	}
}