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

int current_pid = 0; //!< The PID of the currently running process

int tasking = 0; //!< Has multitasking started yet?

int next_kernel_pid = -1;
int next_user_pid   =  1;

lock_t creat_task = 0; //!< Lock used when creating tasks

void proc_jump_to_ring(void);

int proc_by_pid(int pid)
{
	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].pid == pid) return i;
	return -1;
}

int get_pid()
{
	return current_pid;
}

struct kproc procs[MAX_PROCESSES]; //!< Processes

static int get_next_open_proc()
{
	int p = 0;
	while(procs[p].type & TYPE_VALID)
	{
		p++;
		if(p >= MAX_PROCESSES) return -1;
	}
	return p;
}

int add_kernel_task(void *process, char *name, u32 stack_size, int pri)
{
	return add_task(process, name, stack_size, pri, clone_kpagedir(), 1, 0);
}

int add_kernel_task_pdir(void *process, char *name, u32 stack_size, int pri, u32 *pagedir)
{
	return add_task(process, name, stack_size, pri, pagedir, 1, 0);
}

int add_user_task_pdir(void *process, char *name, u32 stack_size, int pri, u32 *pagedir)
{
	return add_task(process, name, stack_size, pri, pagedir, 0, 3);
}




int add_task(void *process, char* name, uint32_t stack_size, int pri, uint32_t *pagedir, int kernel, int ring) {
	lock(&creat_task);
	
	//kerror(ERR_BOOTINFO, "mtask:add_task(%08X, %s, %dK, %d, %08X, %d, %d)", process, name, (stack_size ? (stack_size / 1024) : (STACK_SIZE / 1024)), pri, pagedir, kernel, ring);

	if(ring > 3 || ring < 0) { kerror(ERR_MEDERR, "mtask:add_task: Ring is out of range (0-3): %d", ring); return 0; }

	int parent = 0;
	if(tasking) parent = proc_by_pid(current_pid);

	int p = get_next_open_proc();
	if(p == -1) kerror(ERR_MEDERR, "mtask:add_task: Too many processes, could not create new one.s");

	if(tasking)
	{
		int i = 0;
		for(; i < MAX_CHILDREN; i++)
		{
			if(!procs[parent].children[i])
			{
				procs[parent].children[i] = p;
				break;
			}
		}
		if(i == MAX_CHILDREN)
		{
			kerror(ERR_SMERR, "mtask:add_task: Process %d has run out of children slots", procs[parent].pid);
			unlock(&creat_task);
			return 0;
		}
	}

	memset(&procs[p], 0, sizeof(struct kproc));

	memcpy(procs[p].name, name, strlen(name));

	if(kernel) procs[p].pid = next_kernel_pid--;
	else       procs[p].pid = next_user_pid++;

	// TODO:
	procs[p].uid  = 0;
	procs[p].gid  = 0;

	procs[p].type = TYPE_RUNNABLE | TYPE_VALID | TYPE_RANONCE;

	if(kernel) procs[p].type |= TYPE_KERNEL;

	procs[p].prio = pri;

#if defined(ARCH_X86)
	procs[p].ring = ring;
	procs[p].eip  = (u32)process;
	procs[p].entrypoint = (u32)process;
	procs[p].cr3  = (u32)pagedir;

	uint32_t stack_begin, virt_stack_begin;
	if(!kernel) virt_stack_begin = 0xFF000000;
	else        virt_stack_begin = 0x7F000000;

#ifdef STACK_PROTECTOR
	stack_begin = (u32)kmalloc((stack_size ? stack_size : STACK_SIZE) + 0x2000);
	procs[p].ebp = virt_stack_begin + 0x1000;
	procs[p].ebp +=             ((stack_size ? stack_size : STACK_SIZE) + 0x1000);
#else // STACK_PROTECTOR
	stack_begin = ((u32)kmalloc((stack_size ? stack_size : STACK_SIZE)) & ~0x10) + 0x10;
	procs[p].ebp = virt_stack_begin;
	procs[p].ebp +=              ((stack_size ? stack_size : STACK_SIZE)) & ~0x10;
#endif // !STACK_PROTECTOR

	//procs[p].ebp += 0x10; procs[p].ebp &= 0xFFFFFFF0; // Small alignment


	procs[p].esp = procs[p].ebp;

	u32 i = 0;
	for(; i < (stack_size ? stack_size : STACK_SIZE); i+= 0x1000)
	{
		if(kernel) pgdir_map_page(pagedir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x03);
		else       pgdir_map_page(pagedir, (void *)(stack_begin + i), (void *)(virt_stack_begin + i), 0x07);
			//(void *)(procs[p].esp - i), (void *)(procs[p].esp - i), 0x03);
	}

	procs[p].kernel_stack = (uint32_t)kmalloc(PROC_KERN_STACK_SIZE) + PROC_KERN_STACK_SIZE;
	for(i = 0; i < PROC_KERN_STACK_SIZE; i+=0x1000) {
		pgdir_map_page(pagedir, (void *)(procs[p].kernel_stack + i), (void *)(procs[p].kernel_stack + i), 0x03);
	}

	procs[p].stack_end = procs[p].ebp - (stack_size ? stack_size : STACK_SIZE);
	procs[p].stack_beg = procs[p].ebp;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	block_page(procs[p].stack_end - 0x1000); // <-- Problematic line
	block_page(procs[p].stack_beg + 0x1000);
#endif // STACK_PROTECTOR


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

	//kerror(ERR_BOOTINFO, "PID: %d EIP: %08X CR3: %08X ESP: %08X", procs[p].pid, procs[p].eip, procs[p].cr3, procs[p].esp);

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
			enter_ring(procs[c_proc].ring, (void *)procs[c_proc].entrypoint);
		}
#endif
	}
}




#ifdef ARCH_X86
extern void sched_run(void);
#endif

void init_multitasking(void *process, char *name)
{
	kerror(ERR_BOOTINFO, "Initializing multitasking");

	add_kernel_task(process, name, 0x10000, PRIO_KERNEL);

	kerror(ERR_BOOTINFO, "--");

	procs[0].type &= (u32)~(TYPE_RANONCE); // Don't save registers right away for the first task

	set_interrupt(SCHED_INT, sched_run);

	current_pid = -1;
	tasking = 1;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
}

void run_sched(void)
{
	INTERRUPT(SCHED_INT);
}


__hot void do_task_switch()
{
	if(!tasking)   return;
	if(creat_task) return; // We don't want to interrupt process creation


#if  defined(ARCH_X86)
	u32 esp, ebp, eip, cr3;
	asm volatile ("mov %%esp, %0" : "=r" (esp));
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = (u32)get_eip();

	if(eip == 0xFFFFFFFF)
	{
		sched_processes();
		return;
	}
#endif


	if(procs[c_proc].type & TYPE_RANONCE)
	{
#if  defined(ARCH_X86)
		procs[c_proc].esp = esp;
		procs[c_proc].ebp = ebp;
		procs[c_proc].eip = eip;
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
				 : "%ebx", "%esp", "%eax");
#endif
}


void exit(int code)
{
	int p = proc_by_pid(current_pid);
	if(p == -1)
	{
		kerror(ERR_MEDERR, "Could not find process by pid (%d)", current_pid);
		enable_interrupts();
		run_sched();
	}
	procs[p].type &= (u32)~(TYPE_RUNNABLE);
	procs[p].type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
	procs[p].exitcode = code;

	for(;;)
	{
		run_sched();
	}
}
