#include <arch/mm/alloc.h>

#include <proc/ktasks.h>
#include <err/error.h>
#include <time/time.h>
#include <fs/stream.h>
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
static int ls(int, char **);

struct kterm_entry {
	uint32_t   hash;
	char *string;
	int (*function)(int, char **);
};

struct kterm_entry kterm_ents[] = {
	{ 0, "help",   &help   },
	{ 0, "load",   &load   },
	{ 0, "run",    &run    },
	{ 0, "unload", &unload },
	{ 0, "ls",     &ls     },
};

uint32_t hash(char *str) {
	uint32_t hash = 5381;
	while (*str)
		hash = ((hash << 5) + hash) + (uint8_t)*str++;

	return hash;
}

__noreturn void kterm_task() {
	uint32_t i = 0;
	for(; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++)
		kterm_ents[i].hash = hash(kterm_ents[i].string);

	delay(100); // Wait for things to be initialized

	int retval = 0;

	for(;;) {
		kprintf(prompt);

		char input[512];
		char *argv[32];
		int iloc = 0;

		memset(input, 0, 512);
		memset(argv,  0, sizeof(char *)*32);

		char t = 0;
		while(1) {
			int ret;
			struct ipc_message_user umsg;
			while((ret = ipc_user_recv_message_blocking(&umsg)) < 0) {
				kerror(ERR_MEDERR, "KTERM: IPC error: %d", ret);
			}

			if(umsg.length > sizeof(char)) {
				ipc_user_delete_message(umsg.message_id);
				continue;
			}

			ipc_user_copy_message(umsg.message_id, &t);

			if(t == '\n' || t == '\r') break;
			if(t == '\b') {
				iloc--;
				input[iloc] = ' ';
				kprintf("\b");
				continue;
			}

			if(t != 0) {
				input[iloc++] = t;
				kprintf("%c", t);
			}
		}

		kprintf("\n");

		uint32_t inlen   = strlen(input);
		int cstring = 0;
		int argidx  = 0;
		for(i = 0; i <= inlen; i++) {
			if(input[i] == ' ' || input[i] == '\t' || input[i] == '\0') {
				argv[argidx++] = &input[cstring];
				input[i] = 0;
				cstring = i+1;
			}
		}



		//kprintf("%s\n", input);
		uint32_t h = hash(argv[0]);
		int fnd = 0;
		for(i = 0; i < (sizeof(kterm_ents)/sizeof(kterm_ents[0])); i++) {
			if(kterm_ents[i].hash == h)
				if(!strcmp(kterm_ents[i].string, argv[0])) {
					retval = kterm_ents[i].function(argidx, argv);
					fnd = 1;
					break;
				}
		}
		if(!fnd)
			kprintf("Could not find command: %s\n", argv[0]);
	}
}


static int help(int argc, char **argv) {
	(void)argc;
	(void)argv;
	kprintf("Kterm help:\n");
	kprintf("    help:   display this page\n");
	kprintf("    load:   load an executable\n");
	kprintf("    run:    run a loaded executable\n");
	kprintf("    unload: unload a loaded executable\n");
	kprintf("    ls:     list files in current directory\n");

	return 0;
}


/*
 * Executable file functions:
 */

#define EXEC_BIN 0
#define EXEC_ELF 1

#define EXEC_STREAM_LEN 256

char exec_filename[128] = { 0, };
struct kfile *exec      = NULL;
uint8_t			 *exec_data = NULL;
int           exec_type = 0;

static int load(int argc, char **argv) {
	if(argc < 2) {
		kprintf("No executable specified\n");
		return 1;
	}

	memcpy(exec_filename, argv[1], strlen(argv[1]));

	exec = fs_finddir(fs_root, argv[1]);
	if(!exec) {
		memset(exec_filename, 0, 128);
		kprintf("Could not open %s!\n", argv[1]);
		return 1;
	}
	fs_open(exec, OFLAGS_OPEN | OFLAGS_READ);

	exec_data = kmalloc(exec->length);
	fs_read(exec, 0, exec->length, exec_data);

	//kprintf("First 4 bytes of file: %02x, %02x, %02x, %02x\n", exec_data[0], exec_data[1], exec_data[2], exec_data[3]);

	if(*(uint32_t *)exec_data == ELF_IDENT) {
		exec_type = EXEC_ELF;
		kprintf("Executable is an ELF executable.\n");
	} else {
		exec_type = EXEC_BIN;
		kprintf("Executable is a raw binary executable.\n");
	}

	kprintf("%s loaded\n", exec->name);

	return 0;
}

static int run(int argc, char **argv) {
	(void)argc;
	(void)argv;

	int pid = 0;

	if(!strlen(exec_filename)) {
		kprintf("No loaded executable to run\n");
		return 1;
	}

	if(exec_type == EXEC_ELF) {
		uint32_t *pagedir;
		//ptr_t exec_ep =

		/*uint32_t args[1] = {0};
		call_syscall(SYSCALL_FORK, args);
		int _pid = (int)args[0];*/

		/*if(_pid == 0) {
			load_elf(exec_data, exec->length, &pagedir);
		} else {
			pid = _pid;
		}*/

		pid = load_elf(exec_data, exec->length, &pagedir);

		if(!pid) {
			kerror(ERR_MEDERR, "Could not load executable");
			return 1;
		}

		//pid = add_kernel_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_USERPROG, pagedir);
		//pid = add_user_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_USERPROG, pagedir);
	}

	else if(exec_type == EXEC_BIN) {
		//uint32_t *pagedir;
		//ptr_t exec_ep = load_elf(exec_data, exec->length, &pagedir);
		ptr_t exec_ep = 0x80000000;

		// Keep it on a page boundry:
		void *phys = kmalloc(exec->length + 0x2000);
		phys = (void *)(((uint32_t)phys & ~0xFFF) + 0x1000);

		//map_page(phys, (void *)exec_ep, 0x03);

		//uint32_t addr_tst = (uint32_t)get_page_entry((void *)exec_ep);
		//kerror(ERR_SMERR, "Page entry: 0x%08X", addr_tst);
		
		memcpy(phys, exec_data, exec->length);

		//kerror(ERR_BOOTINFO, "Current CR3: 0x%08X", get_pagedir());

		uint32_t *pagedir = clone_kpagedir();
		pgdir_map_page(pagedir, phys, (void *)exec_ep, 0x03);
		kerror(ERR_BOOTINFO, "Page entry: 0x%08X", pgdir_get_page_entry(pagedir, (void *)exec_ep));

		//pid = add_kernel_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_USERPROG, pagedir);
		pid = add_user_task_pdir((void *)exec_ep, exec_filename, 0x2000, PRIO_USERPROG, pagedir);
		//int pid = add_kernel_task((void *)exec_ep, exec_filename, 0x2000, PRIO_USERPROG);
		
	}

	kerror(ERR_BOOTINFO, "Task PID: %d", pid);
	
	// TODO: Create helper functions for this:
	int idx = proc_by_pid(pid);
	if(idx < 0) {
		kerror(ERR_MEDERR, "kterm: Could not find spawned process!");
		return 1;
	}

	struct kfile *stdin = stream_create(EXEC_STREAM_LEN);
	if(!stdin) {
		kerror(ERR_MEDERR, "kterm: Could not create STDIN!");
		return 1;
	}

	struct kfile *stdout = stream_create(EXEC_STREAM_LEN);
	if(!stdout) {
		kerror(ERR_MEDERR, "kterm: Could not create STDOUT!");
		return 1;
	}

	struct kfile *stderr = stream_create(EXEC_STREAM_LEN);
	if(!stderr) {
		kerror(ERR_MEDERR, "kterm: Could not create STDERR!");
		return 1;
	}

	fs_open(stdin,  OFLAGS_READ | OFLAGS_WRITE);
	fs_open(stdout, OFLAGS_READ | OFLAGS_WRITE);
	fs_open(stderr, OFLAGS_READ | OFLAGS_WRITE);

	procs[idx].open_files[0] = stdin;
	procs[idx].open_files[1] = stdout;
	procs[idx].open_files[2] = stderr;

	char buffer[EXEC_STREAM_LEN];

	while(!(procs[idx].type & TYPE_ZOMBIE)) {
		char t;
		struct ipc_message_user umsg;

		if(ipc_user_recv_message(&umsg) >= 0) {
			if(umsg.length > sizeof(char)) {
				ipc_user_delete_message(umsg.message_id);
			} else {
				ipc_user_copy_message(umsg.message_id, &t);
				fs_write(stdin, 0, 1, (uint8_t *)&t);
				kput(t); // TODO: Handle this better		
			}
		}

		

		if(stdout->length > 0) {
			int sz = fs_read(stdout, 0, stdout->length, (uint8_t *)&buffer);
			for(int i = 0; i < sz; i++) {
				kput(buffer[i]);
			}
		}

		if(stderr->length > 0) {
			int sz = fs_read(stderr, 0, stderr->length, (uint8_t *)&buffer);
			for(int i = 0; i < sz; i++) {
				kput(buffer[i]);
			}
		}
	}

	// Flush remaining output:
	if(stdout->length > 0) {
		int sz = fs_read(stdout, 0, stdout->length, (uint8_t *)&buffer);
		for(int i = 0; i < sz; i++) {
			kput(buffer[i]);
		}
	}

	if(stderr->length > 0) {
		int sz = fs_read(stderr, 0, stderr->length, (uint8_t *)&buffer);
		for(int i = 0; i < sz; i++) {
			kput(buffer[i]);
		}
	}


	return 0;
}

static int unload(int argc, char **argv) {
	(void)argc;
	(void)argv;

	if(!strlen(exec_filename)) {
		kprintf("No loaded executable to unload");
		return 1;
	}

	// TODO: Free allocated data

	memset(exec_filename, 0, 128);

	return 0;
}

static int ls(int argc, char **argv) {
	(void)argc;
	(void)argv;

	
	struct kfile *f;
	DIR *dir;
	struct dirent *d;

	if(argc > 1) {
		// TODO: Allow for more than one directory traversal, and absolute paths
		f = fs_finddir(fs_root, argv[1]);
		if(!f) {
			kprintf("Could not find directory: %s\n", argv[1]);
			return 1;
		}
		dir = fs_opendir(f);
	} else  {
		dir = fs_opendir(fs_root);
	}

	//kprintf("ls: dir: {%08X, %08X, %08X}\n", dir->dir, dir->current, dir->prev);
	
	while((d = fs_readdir(dir))) {
		kfree(d);

		f = fs_dirfile(dir);

		kprintf("%c%c%c%c%c%c%c%c%c%c%s %02d %05d %s\n",
			   ((f->flags & FS_DIR)?'d':'-'),
			   ((f->pflags&0400)?'r':'-'), ((f->pflags&0200)?'w':'-'), ((f->pflags&04000)?'s':((f->pflags&0100)?'x':'-')),
			   ((f->pflags&040)?'r':'-'), ((f->pflags&020)?'w':'-'), ((f->pflags&02000)?'s':((f->pflags&010)?'x':'-')),
			   ((f->pflags&04)?'r':'-'), ((f->pflags&02)?'w':'-'), ((f->pflags&01)?'x':'-'),
			   ((f->pflags&01000)?"T":" "),
			   f->inode, f->length, f->name);
	}
	
	return 0;
}
