#include <proc/mtask.h>
#include <mm/paging.h>
#include <err/error.h>
#include <intr/int.h>
#include <types.h>

void handle_page_fault(u32 errcode, u32 cr3)
{
	kerror(ERR_MEDERR, "Page fault at 0x%08X (%s%s%s%s%s)", cr3,
				((errcode & 0x01) ? "present"                   : "non-present"),
				((errcode & 0x02) ? ", write"                   : ", read"),
				((errcode & 0x04) ? ", user-mode"               : ", kernel-mode"),
				((errcode & 0x08) ? ", modified reserved field" : ""),
				((errcode & 0x10) ? ", instruction fetch"       : ""));

	if(tasking)
	{
		u32 pid = current_pid;
		int p = proc_by_pid(pid);
		if(p == -1)
		{
			kerror(ERR_MEDERR, "Failed to get process index from pid (%d)", pid);
			for(;;);
		}
		
		kerror(ERR_MEDERR, "  -> Caused by process %d", pid);

		if(((cr3 < procs[p].stack_beg) && (cr3 > procs[p].stack_end - STACK_SIZE)) || // Remember, the x86 stack is upside-down
		   ((cr3 < procs[p].stack_beg + STACK_SIZE) && (cr3 > procs[p].stack_end)))
		{
			kerror(ERR_MEDERR, "       -> Caused a stack overflow and is being dealt with", pid);
			exit(0);
			for(;;);
		}
	}

	for(;;);
}