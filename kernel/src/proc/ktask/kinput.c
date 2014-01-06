#include <proc/ktasks.h>
#include <proc/ipc.h>
#include <video.h>

__noreturn void kinput_task()
{
	ktask_pids[KINPUT_TASK_SLOT] = current_pid;
	for(;;)
	{
		char ch;
		recv_message(&ch, sizeof(char)); // TODO: Use an actual message structure instead of simple 8-bit characters
		kprintf("%c", ch);
	}
}