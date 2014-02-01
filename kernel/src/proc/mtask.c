#include <proc/atomic.h>
#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <mm/alloc.h>
#include <string.h>
#include <mm/mm.h>

#if  defined(ARCH_X86)
#include <mm/paging.h>
#include <intr/int.h>
#endif

int current_pid = 0; //!< The PID of the currently running process

int tasking = 0; //!< Has multitasking started yet?

int next_kernel_pid = -1;
int next_user_pid   =  1;

lock_t creat_task = 0; //!< Lock used when creating tasks

int proc_by_pid(int pid)
{
	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].pid == pid) return i;
	return -1;
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

void add_kernel_task(void *process, char *name, u32 stack_size, int pri)
{
	lock(&creat_task);

	int parent = 0;
	if(tasking) parent = proc_by_pid(current_pid);

	int p = get_next_open_proc();
	if(p == -1) kpanic("Could not add a kernel process to process list.");

	if(tasking)
	{
		int i = 0;
		for(; i < MAX_CHILDREN; i++)
		{
			if(procs[parent].children[i]) continue;
			procs[parent].children[i] = p;
			break;
		}
		if(i == MAX_CHILDREN)
		{
			kerror(ERR_MEDERR, "Process %d has run out of child slots", procs[parent].pid);
			unlock(&creat_task);
			return;
		}
	}


	memcpy(procs[p].name, name, strlen(name) + 1);
	procs[p].pid  = next_kernel_pid--;
	procs[p].uid  = 0;
	procs[p].gid  = 0;
	procs[p].type = TYPE_RUNNABLE | TYPE_KERNEL | TYPE_VALID | TYPE_RANONCE;

	procs[p].prio = pri;

#if  defined(ARCH_X86)
	procs[p].eip = (u32)process;
	procs[p].cr3 = kernel_cr3;


#ifdef STACK_PROTECTOR
	procs[p].ebp = (u32)kmalloc((stack_size ? stack_size : STACK_SIZE) + 0x2000);
	procs[p].ebp +=            ((stack_size ? stack_size : STACK_SIZE) + 0x1000);
#else // STACK_PROTECTOR
	procs[p].ebp = (u32)kmalloc((stack_size ? stack_size : STACK_SIZE));
	procs[p].ebp +=            ((stack_size ? stack_size : STACK_SIZE));
#endif // !STACK_PROTECTOR

	//procs[p].ebp += 0x10; procs[p].ebp &= 0xFFFFFFF0; // Small alignment

	procs[p].esp = procs[p].ebp;

	
	procs[p].stack_end = procs[p].ebp - STACK_SIZE;
	procs[p].stack_beg = procs[p].ebp;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	//block_page(procs[p].stack_end - 0x1000); // <-- Problematic line
	//block_page(procs[p].stack_beg + 0x1000);
#endif // STACK_PROTECTOR

#endif // ARCH_X86

// Set up message buffer
	procs[p].messages.head  = 0;
	procs[p].messages.tail  = 0;
	procs[p].messages.count = 0;
	procs[p].messages.size  = MSG_BUFF_SIZE;
	procs[p].messages.buff  = procs[p].msg_buff;

	unlock(&creat_task);

	kerror(ERR_INFO, "Added process %s as pid %d to slot %d", name, procs[p].pid, p);
}

void add_kernel_task_pdir(void *process, char *name, u32 stack_size, int pri, u32 *pagedir)
{
	lock(&creat_task);

	int parent = 0;
	if(tasking) parent = proc_by_pid(current_pid);

	int p = get_next_open_proc();
	if(p == -1) kerror(ERR_SMERR, "Could not add kernel task to the process list");

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
			kerror(ERR_SMERR, "Process %d has run out of children slots", procs[parent].pid);
			unlock(&creat_task);
			return;
		}
	}

	memcpy(procs[p].name, name, strlen(name));

	procs[p].pid  = next_kernel_pid--;
	procs[p].uid  = 0;
	procs[p].gid  = 0;
	procs[p].type = TYPE_RUNNABLE | TYPE_KERNEL | TYPE_VALID | TYPE_RANONCE;
	procs[p].prio = pri;

#if defined(ARCH_X86)
	procs[p].eip = (u32)process;
	procs[p].cr3 = (u32)pagedir;

#ifdef STACK_PROTECTOR
	procs[p].ebp = (u32)kmalloc((stack_size ? stack_size : STACK_SIZE) + 0x2000);
	procs[p].ebp +=            ((stack_size ? stack_size : STACK_SIZE) + 0x1000);
#else // STACK_PROTECTOR
	procs[p].ebp = ((u32)kmalloc((stack_size ? stack_size : STACK_SIZE)) & ~0x10) + 0x10;
	procs[p].ebp +=            ((stack_size ? stack_size : STACK_SIZE)) & ~0x10;
#endif // !STACK_PROTECTOR

	//procs[p].ebp += 0x10; procs[p].ebp &= 0xFFFFFFF0; // Small alignment

	procs[p].esp = procs[p].ebp;

	
	procs[p].stack_end = procs[p].ebp - STACK_SIZE;
	procs[p].stack_beg = procs[p].ebp;

#ifdef STACK_PROTECTOR
// TODO: Fix stack guarding:
	//block_page(procs[p].stack_end - 0x1000); // <-- Problematic line
	//block_page(procs[p].stack_beg + 0x1000);
#endif // STACK_PROTECTOR

#endif // ARCH_X86

// Set up message buffer
	procs[p].messages.head  = 0;
	procs[p].messages.tail  = 0;
	procs[p].messages.count = 0;
	procs[p].messages.size  = MSG_BUFF_SIZE;
	procs[p].messages.buff  = procs[p].msg_buff;



	unlock(&creat_task);
}

void init_multitasking(void *process, char *name)
{
	disable_interrupts();
	kerror(ERR_BOOTINFO, "Initializing multitasking");

	add_kernel_task(process, name, 0x10000, PRIO_KERNEL);

	kerror(ERR_BOOTINFO, "--");

	procs[0].type &= (u32)~(TYPE_RANONCE); // Don't save registers right away for the first task

	tasking = 1;
	current_pid = -1;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
	enable_interrupts();
}


static int c_proc = 0;
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
		return;
	}
	procs[p].type &= (u32)~(TYPE_RUNNABLE);
	procs[p].type |= TYPE_ZOMBIE; // It isn't removed unless it's parent inquires on it
	procs[p].exitcode = code;
	
	for(;;) busy_wait();
}
