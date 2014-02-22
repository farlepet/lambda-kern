#include <proc/ktasks.h>
#include <time/time.h>
#include <proc/ipc.h>
#include <string.h>
#include <video.h>

static char *prompt = "\nkterm> ";

static void help(void);

struct kterm_entry
{
	u32   hash;
	char *string;
	void (*function)(void);
};

struct kterm_entry kterm_ents[] =
{
	{ 0, "help", &help },
};

u32 hash(char *str)
{
	u32 hash = 5381;
	while (*str)
		hash = ((hash << 5) + hash) + (u8)*str++;

	return hash;
}

__noreturn void kterm_task()
{
	ktask_pids[KTERM_TASK_SLOT] = current_pid;

	u32 i = 0;
	for(; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++)
		kterm_ents[i].hash = hash(kterm_ents[i].string);

	delay(100); // Wait for things to be initialized

	for(;;)
	{
		kprintf(prompt);

		char input[512];
		int iloc = 0;

		memset(input, 0, 512);

		char t = 0;
		while(1)
		{
			recv_message(&t, sizeof(char));
			if(t == '\n' || t == '\r') break;

			input[iloc++] = t;
			kprintf("%c", t);
		}

		kprintf("\n");
		//kprintf("%s\n", input);
		u32 h = hash(input);
		int fnd = 0;
		for(i = 0; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++)
		{
			if(kterm_ents[i].hash == h)
				if(!strcmp(kterm_ents[i].string, input))
				{
					kterm_ents[i].function();
					fnd = 1;
					break;
				}
		}
		if(!fnd)
			kprintf("Could not find command\n");
	}
}


static void help()
{
	kprintf("Kterm help:\n");
	kprintf("    help: display this page\n");
}
