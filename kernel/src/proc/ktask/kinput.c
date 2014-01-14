#include <proc/ktasks.h>
#include <err/error.h>
#include <io/input.h>
#include <proc/ipc.h>
#include <video.h>

static char keytab_x86_a[2][256] =
{
	{ // Lowercase
	//    0     1   2    3    4    5    6    7    8    9    A    B    C    D     E     F
	      0 ,  0 , '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',    //0
		 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a',  's',   //1
		 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',  0 ,'\\', 'z', 'x',  'c',  'v',  //2
		 'b', 'n', 'm', ',', '.', '/',  0 ,  0 ,  0 , ' ',  0 ,  0 ,  0 ,  0 ,   0 ,   0 , //3
	},
	{ // Uppercase
	//    0     1   2    3    4    5    6    7    8    9    A    B    C    D     E     F
	      0 ,  0 , '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',    //0
		 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0 ,  'A',  'S',   //1
		 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',  0 , '|', 'Z', 'X',  'C',  'V',  //2
		 'B', 'N', 'M', '<', '>', '?',  0 ,  0 ,  0 , ' ',  0 ,  0 ,  0 ,  0 ,   0 ,   0 , //3
	}
};

static char keycode_to_char(struct input_event *iev)
{
	u32 code = iev->data;

	if(iev->origin.s.driver == IDRIVER_KEYBOARD)
	{
		// TODO: Keep a cache or something so we don't have to do this EVERY time
		struct input_dev *idev = get_idevice(IDRIVER_KEYBOARD, iev->origin.s.device);
		if(!idev)
		{
			kprintf("Could not find device entry\n");
			return '\0';
		}

		// TODO: Check if keyboard keymap is not the default one
		int shift = ((idev->state & KEYB_STATE_SHIFT) == KEYB_STATE_SHIFT);

		return keytab_x86_a[shift][code];
	}

	return '\0';
}

__noreturn void kinput_task()
{
	ktask_pids[KINPUT_TASK_SLOT] = current_pid;
	for(;;)
	{
		struct input_event iev;
		recv_message(&iev, sizeof(struct input_event));
		if(iev.type == EVENT_KEYPRESS)
		{
			if(iev.data == 0x01) // ESC -> DEBUG for now
			{
				struct kbug_type_msg ktm;
				ktm.pid  = current_pid;
				ktm.type = KBUG_IDEBUG;
				while(!ktask_pids[KBUG_TASK_SLOT]);
				send_message(ktask_pids[KBUG_TASK_SLOT], &ktm, sizeof(struct kbug_type_msg));
			}
			else kprintf("%c", keycode_to_char(&iev));
		}
	}
}