#include <proc/mtask.h>
#include <mm/paging.h>
#include <err/error.h>
#include <err/panic.h>
#include <intr/int.h>
#include <types.h>

void handle_page_fault(u32, u32, u32);

/**
 * C side of page fault handler.
 *
 * @param errcode errorcode pushed on stack by the fault
 * @param cr3 value of cr3 register (location of fault)
 */
void handle_page_fault(u32 errcode, u32 cr2, u32 eip)
{
	u32 *cr3 = get_pagedir();
	kerror(ERR_MEDERR, "Page fault at 0x%08X --> 0x%08X (%s%s%s%s%s)", cr2, pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFFFF000,
				((errcode & 0x01) ? "present"                   : "non-present"),
				((errcode & 0x02) ? ", write"                   : ", read"),
				((errcode & 0x04) ? ", user-mode"               : ", kernel-mode"),
				((errcode & 0x08) ? ", modified reserved field" : ""),
				((errcode & 0x10) ? ", instruction fetch"       : ""));

	kerror(ERR_MEDERR, "  -> EIP: %08X", eip);


	if(cr2 >= (u32)firstframe)
	{
		int frame = (cr2 - (u32)firstframe) / 0x1000;
		kerror(ERR_MEDERR, "  -> On frame %08X(%d)", frame, frame);
	}
	else kerror(ERR_MEDERR, "  -> Occurred in kernel-space, not in the page frames");

	kerror(ERR_MEDERR, "  -> Page flags: 0x%03X", pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFF);
	kerror(ERR_MEDERR, "  -> Page Directory: 0x%08X", cr3);
	kerror(ERR_MEDERR, "  -> Kernel pagedir: 0x%08X", kernel_cr3);

	if(tasking)
	{
		int pid = current_pid;
		int p = proc_by_pid(pid);
		if(p == -1)
		{
			kerror(ERR_MEDERR, "Failed to get process index from pid (%d)", pid);
			for(;;);
		}

		kerror(ERR_MEDERR, "  -> Caused by process %d", pid);

		if(((cr2 < procs[p].stack_beg) && (cr2 > procs[p].stack_end - STACK_SIZE)) || // Remember, the x86 stack is upside-down
		   ((cr2 < procs[p].stack_beg + STACK_SIZE) && (cr2 > procs[p].stack_end)))
		{
			kerror(ERR_MEDERR, "       -> Caused a stack overflow and is being dealt with", pid);
			exit(1);
		}
		exit(1);
	}

	kpanic("Page fault, multitasking not enabled, nothing to do to fix this.");

	for(;;);
}


void stub_error()
{
	kerror(ERR_MEDERR, "stub_error() has been called");

	//for(;;);
}
