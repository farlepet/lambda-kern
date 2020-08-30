#include <proc/ktasks.h>
#include <err/error.h>
#include <io/input.h>
#include <proc/ipc.h>
#include <string.h>
#include <video.h>

#include <arch/io/serial.h>

static int input_subs[KINPUT_MAX_SUBS];

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
	uint32_t code = iev->data;

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



static void send_input_char(char c) {
	for(int i = 0; i < KINPUT_MAX_SUBS; i++) {
		if(input_subs[i]) {
			if(proc_by_pid(input_subs[i]) < 0) {
				// Remove dead PID:
				input_subs[i] = 0;
			} else {
				ipc_user_create_and_send_message(input_subs[i], &c, sizeof(char));
			}
		}
	}
}

static int add_subscriber(int pid) {
	for(int i = 0; i < KINPUT_MAX_SUBS; i++) {
		if(!input_subs[i]) {
			input_subs[i] = pid;
			return i;
		}
	}
	return -1;
}


// TODO: Default to 0, add kernel flag to set it to 1, or something else
static int to_kterm = 1; //!< When 1, send all serial input to kterm

__noreturn void kinput_task() {
	ktask_pids[KINPUT_TASK_SLOT] = current_pid;

	if(strlen((const char *)boot_options.init_executable)) {
		/* Make parent of init task a subscriber */
		add_subscriber(1);
		to_kterm = 0;
	}

	for(;;) {
		struct input_event iev;
		recv_message(&iev, sizeof(struct input_event));
		if(iev.type == EVENT_KEYPRESS) {
			if(iev.data == 0x01) { // ESC -> DEBUG for now
				if(ktask_pids[KBUG_TASK_SLOT]) {
					struct kbug_type_msg ktm;
					ktm.type = KBUG_IDEBUG;
					//while(!ktask_pids[KBUG_TASK_SLOT]);
					ipc_user_create_and_send_message(ktask_pids[KBUG_TASK_SLOT], &ktm, sizeof(struct kbug_type_msg));
				}
			}
			// TODO: Send char to some other process
			else {
				if(to_kterm) {
					if(ktask_pids[KTERM_TASK_SLOT] != 0) {
						// Add kterm PID, and if it was added, clear to_kterm
						to_kterm = (add_subscriber(ktask_pids[KTERM_TASK_SLOT]) < 0);
					}
				}
				send_input_char(keycode_to_char(&iev));
			}
		}
		else if(iev.type == EVENT_CHAR) {
			if(to_kterm) {
				if(ktask_pids[KTERM_TASK_SLOT] != 0) {
					// Add kterm PID, and if it was added, clear to_kterm
					to_kterm = (add_subscriber(ktask_pids[KTERM_TASK_SLOT]) < 0);
				}
			}
			send_input_char(iev.data);
		}
	}
}
