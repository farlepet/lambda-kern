#include <proc/ktasks.h>
#include <err/error.h>
#include <io/input.h>
#include <fs/fs.h>
#include <io/output.h>

#include <string.h>

kfile_hand_t *kinput_dest = NULL;

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
    /* @todo Make data available at /dev/... rather than sending directly to a
     * thread. */
    if(kinput_dest) {
        fs_write(kinput_dest, 0, 1, (uint8_t *)&c);
    }
    /* Temporary workaround prior to real TTY devices */
    kput(c);
}


__noreturn void kinput_task() {
    for(;;) {
        struct input_event iev;
        /* @todo Refactor input device driver system, add read block ability to cbuffs */
        input_dev_t     *idev;
        llist_iterator_t iter;
        llist_iterator_init(&idevs, &iter);
        while(llist_iterate(&iter, (void **)&idev)) {
            if(idev->iev_buff) {
                while(cbuff_read((uint8_t *)&iev, sizeof(struct input_event), idev->iev_buff) >= 0) {
#if DEBUGGER
                    if(iev.type == EVENT_KEYPRESS &&
                    iev.data == 0x01) { // ESC -> DEBUG for now
                        if(ktask_pids[KBUG_TASK_SLOT]) {
                            /*struct kbug_type_msg ktm;
                            ktm.type = KBUG_IDEBUG;
                            ipc_user_create_and_send_message(ktask_pids[KBUG_TASK_SLOT], &ktm, sizeof(struct kbug_type_msg));*/
                            /* @todo (Or not) */
                        }
                    } else {
#endif /* DEBUGGER */
                        if(iev.type == EVENT_KEYPRESS) {
                            send_input_char(keycode_to_char(&iev));
                        } else {
                            send_input_char(iev.data);
                        }
#if DEBUGGER
                    }
#endif
                }
            }
        }

        delay(10);
    }
}
