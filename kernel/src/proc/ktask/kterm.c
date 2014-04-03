#include <proc/ktasks.h>
#include <err/error.h>
#include <time/time.h>
#include <mm/alloc.h>
#include <proc/ipc.h>
#include <proc/elf.h>
#include <string.h>
#include <fs/fs.h>
#include <video.h>

#define prompt  "%skterm\e[0m> ", \
				(retval ? "\e[31m" : "\e[32m")

static int help(int, char **);
static int load(int, char **);
static int run(int, char **);
static int unload(int, char **);

struct kterm_entry
{
	u32   hash;
	char *string;
	int (*function)(int, char **);
};

struct kterm_entry kterm_ents[] =
{
	{ 0, "help",   &help   },
	{ 0, "load",   &load   },
	{ 0, "run",    &run    },
	{ 0, "unload", &unload },
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

	int retval = 0;

	for(;;)
	{
		kprintf(prompt);

		char input[512];
		char *argv[32];
		int iloc = 0;

		memset(input, 0, 512);
		memset(argv,  0, sizeof(char *)*32);

		char t = 0;
		while(1)
		{
			recv_message(&t, sizeof(char));
			if(t == '\n' || t == '\r') break;

			input[iloc++] = t;
			kprintf("%c", t);
		}

		kprintf("\n");

		u32 inlen   = strlen(input);
		int cstring = 0;
		int argidx  = 0;
		for(i = 0; i <= inlen; i++)
		{
			if(input[i] == ' ' || input[i] == '\t' || input[i] == '\0')
			{
				argv[argidx++] = &input[cstring];
				input[i] = 0;
				cstring = i+1;
			}
		}



		//kprintf("%s\n", input);
		u32 h = hash(argv[0]);
		int fnd = 0;
		for(i = 0; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++)
		{
			if(kterm_ents[i].hash == h)
				if(!strcmp(kterm_ents[i].string, argv[0]))
				{
					retval = kterm_ents[i].function(argidx, argv);
					fnd = 1;
					break;
				}
		}
		if(!fnd)
			kprintf("Could not find command: %s\n", argv[0]);
	}
}


static int help(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	kprintf("Kterm help:\n");
	kprintf("    help:   display this page\n");
	kprintf("    load:   load an executable\n");
	kprintf("    run:    run a loaded executable\n");
	kprintf("    unload: unload a loaded executable\n");

	return 0;
}


/*
 * Executable file functions:
 */

#define EXEC_BIN 0
#define EXEC_ELF 1

char exec_filename[128] = { 0, };
struct kfile *exec      = NULL;
u8			 *exec_data = NULL;
int           exec_type = 0;

static int load(int argc, char **argv)
{
	if(argc < 2)
	{
		kprintf("No executable specified\n");
		return 1;
	}

	memcpy(exec_filename, argv[1], strlen(argv[1]));

	exec = fs_finddir(fs_root, argv[1]);
	if(!exec)
	{
		memset(exec_filename, 0, 128);
		kprintf("Could not open %s!\n", argv[1]);
		return 1;
	}
	fs_open(exec, OFLAGS_OPEN | OFLAGS_READ);

	exec_data = kmalloc(exec->length);
	fs_read(exec, 0, exec->length, exec_data);

	if(*(u32 *)exec_data == ELF_IDENT)
	{
		exec_type = EXEC_ELF;
		kprintf("Executable is an ELF executable.\n");
		kprintf("NOTE: ELF executables are not fully supported, so don't expect it to work.\n");
	}
	else
	{
		exec_type = EXEC_BIN;
		kprintf("Executable is a raw binary executable.\n");
	}

	kprintf("%s loaded\n", exec->name);

	return 0;
}

static int run(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	if(!strlen(exec_filename))
	{
		kprintf("No loaded executable to run\n");
		return 1;
	}

	if(exec_type == EXEC_ELF)
	{
		u32 *pagedir;
		ptr_t exec_ep = load_elf(exec_data, exec->length, &pagedir);
		if(!exec_ep)
		{
			kerror(ERR_MEDERR, "Could not load executable");
			return 1;
		}

		add_kernel_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_DRIVER, pagedir);
	}

	else if(exec_type == EXEC_BIN)
	{
		//u32 *pagedir;
		//ptr_t exec_ep = load_elf(exec_data, exec->length, &pagedir);
		ptr_t exec_ep = 0x80000000;

		void *phys = kmalloc(exec->length + 0x2000);
		phys = (void *)(((u32)phys & ~0xFFF) + 0x1000);

		map_page(phys, (void *)exec_ep, 0x03);

		memcpy((void *)exec_ep, exec_data, exec->length);

		//add_kernel_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_DRIVER, pagedir);
		add_kernel_task((void *)exec_ep, exec_filename, 0x2000, PRIO_DRIVER);
	}

	//if(pagedir) set_pagedir(pagedir);

	//kprintf("Set page directory\n");


	//void (*prog)() = (void *)exec_ep;
	//prog();

	kprintf("Executable is now running\n");

	return 0;
}

static int unload(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	if(!strlen(exec_filename))
	{
		kprintf("No loaded executable to unload");
		return 1;
	}

	memset(exec_filename, 0, 128);

	return 0;
}
