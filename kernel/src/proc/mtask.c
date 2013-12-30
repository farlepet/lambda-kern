#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <proc/proc.h>
#include <string.h>

#if  defined(ARCH_X86)
#include <mm/paging.h>
#include <intr/int.h>
#endif

int current_pid = 0; //!< The PID of the currently running process

int tasking = 0; //!< Has multitasking started yet?

int next_kernel_pid = -1;
int next_user_pid   =  1;

static struct kproc procs[MAX_PROCESSES]; //!< Processes

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

void add_kernel_task(void *process, char *name)
{
	int ints_en = interrupts_enabled();
	disable_interrupts();

	int p = get_next_open_proc();
	if(p == -1) kpanic("Could not add a kernel process to process list.");

	memcpy(procs[p].name, name, strlen(name) + 1);
	procs[p].pid  = next_kernel_pid--;
	procs[p].uid  = 0;
	procs[p].gid  = 0;
	procs[p].type = TYPE_RUNNABLE | TYPE_KERNEL | TYPE_VALID;

#if  defined(ARCH_X86)
	procs[p].eip = (u32)process;
	procs[p].cr3 = kernel_cr3;
	procs[p].ebp = (u32)kmalloc(STACK_SIZE) + STACK_SIZE;
	procs[p].esp = procs[p].ebp;
#endif

	if(ints_en) enable_interrupts();
	kerror(ERR_INFO, "Added process %s as pid %d to slot %d", name, procs[p].pid, p);
}

void init_multitasking(void *process, char *name)
{
	disable_interrupts();

	add_kernel_task(process, name);

	tasking = 1;
	current_pid = -1;

	kerror(ERR_BOOTINFO, "Multitasking enabled");
	enable_interrupts();
}


int proc_by_pid(int pid)
{
	int i = 0;
	for(; i < MAX_PROCESSES; i++)
		if(procs[i].pid == pid) return i;
	return -1;
}

static int c_proc = 0;
void do_task_switch()
{
	if(!tasking) return;

	//kerror(ERR_BOOTINFO, "do_task_switch()");

#if  defined(ARCH_X86)
	u32 esp, ebp, eip, cr3;
	asm volatile ("mov %%esp, %0" : "=r" (esp));
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	eip = (u32)get_eip();

	if(eip == 0xFFFFFFFF) return;
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
	c_proc++;
	while(!(procs[c_proc].type & TYPE_RUNNABLE)) { c_proc++; if(c_proc >= MAX_PROCESSES) c_proc = 0; }
	current_pid = procs[c_proc].pid;


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