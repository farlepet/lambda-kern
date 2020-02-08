#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <intr/int.h>
#include <mm/alloc.h>
#include <string.h>
#include <mm/mm.h>
#include <video.h>
#include <fs/fs.h>

#if  defined(ARCH_X86)
#include <mm/paging.h>
#include <intr/int.h>
#include <proc/user.h>
#include <mm/gdt.h>
#endif

struct kproc procs[MAX_PROCESSES];

int current_pid = 0; //!< The PID of the currently running process

int tasking = 0; //!< Has multitasking started yet?

int next_kernel_pid = -1;
int next_user_pid   =  1;

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

int get_next_pid(int kernel) {
	if(kernel) return next_kernel_pid--;
	else       return next_user_pid++;
}

struct kproc procs[MAX_PROCESSES]; //!< Processes

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
#if defined(ARCH_X86)
	uintptr_t stack_begin = (uintptr_t)kmalloc(stack_size + 4096);
	stack_begin = (stack_begin + 4096) & 0xFFFFF000;

	kerror(ERR_BOOTINFO, "proc_create_stack [size: %d] [end: %08X, beg: %08X]",
		stack_size, stack_begin, stack_begin + stack_size
	);

/*#ifdef STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size + 0x2000);
#else // STACK_PROTECTOR
	stack_begin = (uintptr_t)kmalloc(stack_size);
#endif // !STACK_PROTECTOR*/

	for(size_t i = 0; i < stack_size; i+= 0x1000) {
		if(is_kernel) pgdir_map_page((uint32_t *)proc->cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
		else          pgdir_map_page((uint32_t *)proc->cr3, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
	}

	proc->stack_end = virt_stack_begin;
	proc->stack_beg = virt_stack_begin + stack_size;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	block_page(proc->stack_end - 0x1000);
	block_page(proc->stack_beg);
#endif // STACK_PROTECTOR
#endif // ARCH_X86

	return 0;
}

int proc_create_kernel_stack(struct kproc *proc) {
#if defined(ARCH_X86)
	proc->kernel_stack = (uint32_t)kmalloc(PROC_KERN_STACK_SIZE + 4096);
	proc->kernel_stack = (proc->kernel_stack + 4096) & 0xFFFFF000;

	kerror(ERR_BOOTINFO, "proc_create_kernel_stack [size: %d] [end: %08X, beg: %08X]",
		PROC_KERN_STACK_SIZE, proc->kernel_stack, proc->kernel_stack + PROC_KERN_STACK_SIZE
	);

	for(size_t i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
		pgdir_map_page((uint32_t *)proc->cr3, (void *)(proc->kernel_stack + i), (void *)(proc->kernel_stack + i), 0x03);
	}

	proc->kernel_stack += PROC_KERN_STACK_SIZE;
#endif // ARCH_X86

	return 0;
}


int add_task(void *process, char* name, uint32_t stack_size, int pri, uint32_t *pagedir, int kernel, int ring) {
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
			if(!procs[parent].children[i]) {
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

	procs[p].pid = get_next_pid(kernel);

	// TODO:
	procs[p].uid  = 0;
	procs[p].gid  = 0;

	procs[p].type = TYPE_RUNNABLE | TYPE_VALID | TYPE_RANONCE;

	if(kernel) procs[p].type |= TYPE_KERNEL;

	procs[p].prio = pri;

#if defined(ARCH_X86)
	procs[p].ring       = ring;
	procs[p].eip        = (uint32_t)process;
	procs[p].entrypoint = (uint32_t)process;
	procs[p].cr3        = (uint32_t)pagedir;

	uint32_t /*stack_begin, */virt_stack_begin;
	if(!kernel) virt_stack_begin = 0xFF000000;
	else        virt_stack_begin = 0x7F000000;

	proc_create_stack(&procs[p], stack_size, virt_stack_begin, kernel);
	proc_create_kernel_stack(&procs[p]);

	procs[p].esp = procs[p].ebp = virt_stack_begin + stack_size;

	if(kernel == 0) {
		/*STACK_PUSH(stack_begin, 0xFFFFAAAA);//process);
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		STACK_PUSH(stack_begin, ring);
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		STACK_PUSH(stack_begin, 0xFEAFBEEF); // Return Address
		kerror(ERR_BOOTINFO, "STACK_PEEK[%08X]: %08X", stack_begin, *(uint32_t *)stack_begin);
		procs[p].eip = (uint32_t)enter_ring;
		procs[p].esp -= 12;*/
		procs[p].eip = (uint32_t)proc_jump_to_ring;
	}

#endif // ARCH_X86


// Set up message buffer
	procs[p].messages.head  = 0;
	procs[p].messages.tail  = 0;
	procs[p].messages.count = 0;
	procs[p].messages.size  = MSG_BUFF_SIZE;
	procs[p].messages.buff  = procs[p].msg_buff;

	procs[p].cwd = fs_root;

	kerror(ERR_BOOTINFO, "PID: %d EIP: %08X CR3: %08X ESP: %08X", procs[p].pid, procs[p].eip, procs[p].cr3, procs[p].esp);

	//uint32_t page = pgdir_get_page_entry(pagedir, process);
	//kerror(ERR_BOOTINFO, "Page %08X: LOC: %08X FLAGS: %03X", process, page & 0xFFFFF000, page & 0x0FFF);

	unlock(&creat_task);

	return procs[p].pid;
}


static int c_proc = 0;

void proc_jump_to_ring(void) {
	int p = proc_by_pid(current_pid);
	if(p > 0) {
#if defined(ARCH_X86)
		if(procs[c_proc].entrypoint) {
			enter_ring_noargs(procs[c_proc].ring, (void *)procs[c_proc].entrypoint);
		}
#endif
	}
}




#ifdef ARCH_X86
extern void sched_run(void);
#endif

void init_multitasking(void *process, char *name) {
	kerror(ERR_BOOTINFO, "Initializing multitasking");

	add_kernel_task(process, name, 0x10000, PRIO_KERNEL);

	kerror(ERR_BOOTINFO, "--");

	procs[0].type &= (uint32_t)~(TYPE_RANONCE); // Don't save registers right away for the first task

	set_interrupt(SCHED_INT, sched_run);

	current_pid = -1;
	tasking = 1;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
}

void run_sched(void) {
	INTERRUPT(SCHED_INT);
}


__hot void do_task_switch(struct pusha_regs pregs, struct iret_regs iregs) {
	if(!tasking)   return;
	if(creat_task) return; // We don't want to interrupt process creation

	(void)pregs;

#if  defined(ARCH_X86)
	uint32_t esp, ebp, eip, cr3;
	asm volatile ("mov %%esp, %0" : "=r" (esp));
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = (uint32_t)get_eip();

	if(eip == 0xFFFFFFFF) {
		sched_processes();
		return;
	}
#endif


	if(procs[c_proc].type & TYPE_RANONCE) {
#if  defined(ARCH_X86)
		procs[c_proc].esp = esp;
		procs[c_proc].ebp = ebp;
		procs[c_proc].eip = eip;

		procs[c_proc].last_eip = iregs.eip;
#endif
	}
	else procs[c_proc].type |= TYPE_RANONCE;

	// Switch to next process here...
	c_proc = sched_next_process();


#if  defined(ARCH_X86)
	tss_set_kern_stack(procs[c_proc].kernel_stack);

	esp = procs[c_proc].esp;
	ebp = procs[c_proc].ebp;
	eip = procs[c_proc].eip;
	cr3 = procs[c_proc].cr3;

	asm volatile("mov %0, %%ebx\n"
				 "mov %1, %%esp\n"
				 "mov %2, %%ebp\n"
				 "mov %3, %%cr3\n"
				 "mov $0xFFFFFFFF, %%eax\n"
				 "sti\n"
				 "jmp *%%ebx"
				 : : "r" (eip), "r" (esp), "r" (ebp), "r" (cr3)
				 : "%ebx", /*"%esp", */"%eax");
#endif
}


__noreturn void exit(int code) {
	int p = proc_by_pid(current_pid);
	if(p == -1) {
		kerror(ERR_MEDERR, "Could not find process by pid (%d)", current_pid);
		enable_interrupts();
		run_sched();
	}
	procs[p].type &= (uint32_t)~(TYPE_RUNNABLE);
	procs[p].type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
	procs[p].exitcode = code;

	for(;;) {
		run_sched();
	}
}
