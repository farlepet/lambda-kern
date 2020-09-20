#include <arch/intr/int.h>
#include <arch/proc/tasking.h>

#include <proc/atomic.h>
#include <proc/mtask.h>
#include <proc/proc.h>
#include <err/error.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <mm/alloc.h>
#include <string.h>
#include <mm/mm.h>
#include <video.h>
#include <fs/fs.h>

#if defined(ARCH_X86)
#  include <arch/mm/paging.h>
#endif

struct kproc procs[MAX_PROCESSES];

int current_pid = 0; //!< The PID of the currently running process

int tasking = 0; //!< Has multitasking started yet?

static int next_pid = 1;

lock_t creat_task = 0; //!< Lock used when creating tasks

void proc_jump_to_ring(void);

int proc_by_pid(int pid) {
	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].pid == pid) return i;
	return -1;
}

int get_pid() {
	return current_pid;
}

int get_next_pid() {
	return next_pid++;
}

int get_next_open_proc() {
	int p = 0;
	while(procs[p].type & TYPE_VALID) {
		p++;
		if(p >= MAX_PROCESSES) return -1;
	}
	return p;
}

int add_kernel_task(void *process, char *name, uint32_t stack_size, int pri) {
	return add_task(process, name, stack_size, pri, clone_kpagedir(), 1, 0);
}

int add_kernel_task_pdir(void *process, char *name, uint32_t stack_size, int pri, uint32_t *pagedir) {
	return add_task(process, name, stack_size, pri, pagedir, 1, 0);
}

int add_user_task(void *process, char *name, uint32_t stack_size, int pri) {
	return add_task(process, name, stack_size, pri, clone_kpagedir(), 0, 3);
}

int add_user_task_pdir(void *process, char *name, uint32_t stack_size, int pri, uint32_t *pagedir) {
	return add_task(process, name, stack_size, pri, pagedir, 0, 3);
}

int proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel) {
	if(arch_proc_create_stack(proc, stack_size, virt_stack_begin, is_kernel)) {
		return 1;
	}

	return 0;
}

int proc_create_kernel_stack(struct kproc *proc) {
	if(arch_proc_create_kernel_stack(proc)) {
		return 1;
	}

	return 0;
}


int add_task(void *process, char* name, uint32_t stack_size, int pri, uint32_t *pagedir, int kernel, int ring) {
	// TODO: Remove reference to ring and page directory

	lock(&creat_task);
	
	if(!stack_size) stack_size = STACK_SIZE;

	//kerror(ERR_BOOTINFO, "mtask:add_task(%08X, %s, %dK, %d, %08X, %d, %d)", process, name, (stack_size ? (stack_size / 1024) : (STACK_SIZE / 1024)), pri, pagedir, kernel, ring);

	if(ring > 3 || ring < 0) { kerror(ERR_MEDERR, "mtask:add_task: Ring is out of range (0-3): %d", ring); return 0; }

	int parent = 0;
	if(tasking) parent = proc_by_pid(current_pid);

	int p = get_next_open_proc();
	if(p == -1) {
		kerror(ERR_MEDERR, "mtask:add_task: Too many processes, could not create new one.");
		return -1;
	}

	if(tasking) {
		int i = 0;
		for(; i < MAX_CHILDREN; i++) {
			if(procs[parent].children[i] < 0) {
				procs[parent].children[i] = p;
				break;
			}
		}
		if(i == MAX_CHILDREN) {
			kerror(ERR_SMERR, "mtask:add_task: Process %d has run out of children slots", procs[parent].pid);
			unlock(&creat_task);
			return 0;
		}
	}

	memset(&procs[p], 0, sizeof(struct kproc));

	memcpy(procs[p].name, name, strlen(name));

	procs[p].pid = get_next_pid();

	// TODO:
	procs[p].uid  = 0;
	procs[p].gid  = 0;

	procs[p].parent = current_pid;

	procs[p].type = TYPE_RUNNABLE | TYPE_VALID | TYPE_RANONCE;

	if(kernel) procs[p].type |= TYPE_KERNEL;

	procs[p].prio = pri;

	arch_setup_task(&procs[p], process, stack_size, pagedir, kernel, ring);

/*#if defined(ARCH_X86)
	/ In case task is expecting these to be populated. /
    STACK_PUSH(procs[p].esp, 0);           // NULL
    STACK_PUSH(procs[p].esp, procs[p].esp);   // ENVP
    STACK_PUSH(procs[p].esp, procs[p].esp+4); // ARGV
    STACK_PUSH(procs[p].esp, 0);           // ARGC
#endif*/

	proc_create_kernel_stack(&procs[p]);

// Set up message buffer
	procs[p].messages.head  = 0;
	procs[p].messages.tail  = 0;
	procs[p].messages.count = 0;
	procs[p].messages.size  = MSG_BUFF_SIZE;
	procs[p].messages.buff  = procs[p].msg_buff;

	procs[p].cwd = fs_root;

	// Set all children to -1 (Assuming 2s complement)
	memset(procs[p].children, 0xFF, sizeof(procs[p].children));

	kerror(ERR_BOOTINFO, "PID: %d EIP: %08X CR3: %08X ESP: %08X", procs[p].pid, procs[p].arch.eip, procs[p].arch.cr3, procs[p].arch.esp);

	//uint32_t page = pgdir_get_page_entry(pagedir, process);
	//kerror(ERR_BOOTINFO, "Page %08X: LOC: %08X FLAGS: %03X", process, page & 0xFFFFF000, page & 0x0FFF);

	unlock(&creat_task);

	return procs[p].pid;
}




void init_multitasking(void *process, char *name) {
	kerror(ERR_BOOTINFO, "Initializing multitasking");

	add_kernel_task(process, name, 0x10000, PRIO_KERNEL);

	kerror(ERR_BOOTINFO, "--");

	procs[0].type &= (uint32_t)~(TYPE_RANONCE); // Don't save registers right away for the first task

	arch_multitasking_init();

	current_pid = -1;
	tasking = 1;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
}



__noreturn void exit(int code) {
	int p = proc_by_pid(current_pid);
	if(p == -1) {
		kerror(ERR_MEDERR, "Could not find process by pid (%d)", current_pid);
		enable_interrupts();
		run_sched();
	}

	kerror(ERR_BOOTINFO, "exit(%d) called by process %d.", code, p);

	// If parent processis waiting for child to exit, allow it to continue execution:
	if(procs[p].parent) {
		int pidx = proc_by_pid(procs[p].parent);
		procs[pidx].blocked &= (uint32_t)~(BLOCK_WAIT);
	}

	procs[p].type &= (uint32_t)~(TYPE_RUNNABLE);
	procs[p].type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
	procs[p].exitcode = code;

	for(;;) {
		run_sched();
	}
}
