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

struct kproc *procs     = NULL;
struct kproc *curr_proc = NULL;

static int next_pid = 1;

lock_t creat_task = 0; //!< Lock used when creating tasks

void proc_jump_to_ring(void);

struct kproc *proc_by_pid(int pid) {
	if(!procs) {
		return NULL;
	}

	struct kproc *proc = procs;
	do {
		if(proc->pid == pid) {
			return proc;
		}
		proc = proc->next;
	} while(proc && proc != procs);
	
	return NULL;
}

int get_pid() {
	if(!curr_proc) {
		return -1;
	} else {
		return curr_proc->pid;
	}
}

int get_next_pid() {
	return next_pid++;
}

int add_kernel_task(void *process, char *name, uint32_t stack_size, int pri) {
	arch_task_params_t arch_params;
#if defined(ARCH_X86)
	arch_params.ring  = 0;
	arch_params.pgdir = clone_kpagedir();
#endif
	return add_task(process, name, stack_size, pri, 1, &arch_params);
}

int add_kernel_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params) {
#if defined(ARCH_X86)
	arch_params->ring = 0;
#endif
	return add_task(process, name, stack_size, pri, 1, arch_params);
}

int add_user_task(void *process, char *name, uint32_t stack_size, int pri) {
	arch_task_params_t arch_params;
#if defined(ARCH_X86)
	arch_params.ring = 3;
	arch_params.pgdir = clone_kpagedir();
#endif
	return add_task(process, name, stack_size, pri, 0, &arch_params);
}

int add_user_task_arch(void *process, char *name, uint32_t stack_size, int pri, arch_task_params_t *arch_params) {
#if defined(ARCH_X86)
	arch_params->ring = 3;
#endif
	return add_task(process, name, stack_size, pri, 0, arch_params);
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


int add_task(void *process, char* name, uint32_t stack_size, int pri, int kernel, arch_task_params_t *arch_params) {
	// TODO: Remove reference to ring

	lock(&creat_task);
	
	if(!stack_size) stack_size = STACK_SIZE;

	//kerror(ERR_BOOTINFO, "mtask:add_task(%08X, %s, %dK, %d, %08X, %d, %d)", process, name, (stack_size ? (stack_size / 1024) : (STACK_SIZE / 1024)), pri, pagedir, kernel, ring);

#if defined(ARCH_X86)
	if(arch_params->ring > 3) { kerror(ERR_MEDERR, "mtask:add_task: Ring is out of range (0-3): %d", arch_params->ring); return 0; }
#endif

	int parent = 0;
	if(curr_proc) {
		parent = curr_proc->pid;
	}

	struct kproc *proc = (struct kproc *)kmalloc(sizeof(struct kproc));
	if(!proc) {
		kerror(ERR_LGERR, "mtask:add_task: Not enough memory to allocate new task.");
		return -1;
	}

	if(procs && proc_add_child(curr_proc, proc)) {
		kerror(ERR_SMERR, "mtask:add_task: Process %d has run out of children slots", curr_proc->pid);
		unlock(&creat_task);
		kfree(proc);
		return 0;
	}

	memset(proc, 0, sizeof(struct kproc));

	memcpy(proc->name, name, strlen(name));

	proc->pid = get_next_pid();

	// TODO:
	proc->uid  = 0;
	proc->gid  = 0;

	proc->parent = curr_proc->pid;

	proc->type = TYPE_RUNNABLE | TYPE_VALID | TYPE_RANONCE;

	if(kernel) proc->type |= TYPE_KERNEL;

	proc->prio = pri;

	arch_setup_task(proc, process, stack_size, kernel, arch_params);

	proc_create_kernel_stack(proc);

	/* Set up message buffer */
	proc->messages.head  = 0;
	proc->messages.tail  = 0;
	proc->messages.count = 0;
	proc->messages.size  = MSG_BUFF_SIZE;
	proc->messages.buff  = proc->msg_buff;

	proc->cwd = fs_root;

	memset(proc->children, 0, sizeof(proc->children));

#if defined(ARCH_X86)
	kerror(ERR_BOOTINFO, "PID: %d EIP: %08X CR3: %08X ESP: %08X", proc->pid, proc->arch.eip, proc->arch.cr3, proc->arch.esp);
#endif

	mtask_insert_proc(proc);

	unlock(&creat_task);

	return proc->pid;
}


int mtask_insert_proc(struct kproc *proc) {
	if(!procs) {
		procs = proc;
		proc->next = proc;
		proc->prev = proc;
	} else {
		/* TODO: Keep track of last proc? */
		struct kproc *last = procs;
		while(last->next && last->next != procs) {
			last = last->next;
		}
		/* Insert after last process: */
		proc->prev  = last;
		proc->next  = procs;
		last->next  = proc;
		procs->prev = proc;
	}

	return 0;
}


struct kproc *mtask_get_current_task(void) {
	return curr_proc;
}


void init_multitasking(void *process, char *name) {
	kerror(ERR_BOOTINFO, "Initializing multitasking");

	int pid = add_kernel_task(process, name, 0x10000, PRIO_KERNEL);
	struct kproc *proc = proc_by_pid(pid);

	proc->type &= (uint32_t)~(TYPE_RANONCE); // Don't save registers right away for the first task

	arch_multitasking_init();

	curr_proc = proc;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
}



__noreturn void exit(int code) {
	kerror(ERR_BOOTINFO, "exit(%d) called by process %d.", code, curr_proc->pid);

	// If parent processis waiting for child to exit, allow it to continue execution:
	if(curr_proc->parent) {
		struct kproc *parent = proc_by_pid(curr_proc->parent);
		parent->blocked &= (uint32_t)~(BLOCK_WAIT);
	}

	curr_proc->type &= (uint32_t)~(TYPE_RUNNABLE);
	curr_proc->type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
	curr_proc->exitcode = code;

	for(;;) {
		run_sched();
	}
}
