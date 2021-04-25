#include <arch/intr/int.h>

#include <proc/ktasks.h>

/**
 * A basic task that only loops, 
 * ran if no other tasks are available
 */
__noreturn void idle_task()
{
	for(;;)
	{
		busy_wait();
	}
}