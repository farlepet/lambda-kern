#include <proc/ktasks.h>
#include <err/error.h>
#include <proc/ipc.h>
#include <video.h>

__noreturn void kinput_task()
{
	ktask_pids[KINPUT_TASK_SLOT] = current_pid;
	for(;;)
	{
		char ch;
		recv_message(&ch, sizeof(char)); // TODO: Use an actual message structure instead of simple 8-bit characters
		if(ch == 0x1B) // ESC -> DEBUG for now
		{
			struct kbug_type_msg ktm;
			ktm.pid  = current_pid;
			ktm.type = KBUG_IDEBUG;
			while(!ktask_pids[KBUG_TASK_SLOT]);
			send_message(ktask_pids[KBUG_TASK_SLOT], &ktm, sizeof(struct kbug_type_msg));
		}
		else kprintf("%c", ch);
	}
}