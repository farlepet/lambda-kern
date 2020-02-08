#include <mm/stack_trace.h>
#include <proc/mtask.h>
#include <mm/paging.h>
#include <err/error.h>
#include <err/panic.h>
#include <intr/int.h>
#include <types.h>
#include <video.h>



__noreturn void handle_page_fault(uint32_t, uint32_t,/* uint32_t *ebp, */struct pusha_regs, struct iret_regs iregs);
__noreturn void handle_gpf(uint32_t errcode, struct pusha_regs regs, struct iret_regs iregs);
__noreturn void handle_invalid_op(struct pusha_regs regs, struct iret_regs iregs);
__noreturn void handle_double_fault(struct pusha_regs regs, uint32_t errcode, struct iret_regs iregs);
static void dump_regs(struct pusha_regs regs);
static void dump_iregs(struct iret_regs iregs);

/**
 * C side of page fault handler.
 *
 * @param errcode errorcode pushed on stack by the fault
 * @param cr3 value of cr3 register (location of fault)
 */
void handle_page_fault(uint32_t errcode, uint32_t cr2,/* uint32_t *ebp, */struct pusha_regs regs, struct iret_regs iregs)
{
	uint32_t *cr3 = get_pagedir();

	kerror(ERR_MEDERR, "Page fault at 0x%08X --> 0x%08X (%s%s%s%s%s)", cr2, pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFFFF000,
				((errcode & 0x01) ? "present"                   : "non-present"),
				((errcode & 0x02) ? ", write"                   : ", read"),
				((errcode & 0x04) ? ", user-mode"               : ", kernel-mode"),
				((errcode & 0x08) ? ", modified reserved field" : ""),
				((errcode & 0x10) ? ", instruction fetch"       : ""));

	dump_iregs(iregs);
	dump_regs(regs);

	if(cr2 >= (uint32_t)firstframe)
	{
		int frame = (cr2 - (uint32_t)firstframe) / 0x1000;
		kerror(ERR_MEDERR, "  -> On frame %08X(%d)", frame, frame);
	}
	else kerror(ERR_MEDERR, "  -> Occurred in kernel-space, not in the page frames");

	kerror(ERR_MEDERR, "  -> Page flags:  0x%03X", pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFF);
	kerror(ERR_MEDERR, "  -> Table flags: 0x%03X", pgdir_get_page_table(cr3, (void *)cr2) & 0xFFF);
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

		kerror(ERR_MEDERR, "  -> Caused by process %d [%s]", pid, procs[p].name);

		if(((cr2 < procs[p].stack_beg) && (cr2 > procs[p].stack_end - STACK_SIZE)) || // Remember, the x86 stack is upside-down
		   ((cr2 < procs[p].stack_beg + STACK_SIZE) && (cr2 > procs[p].stack_end)))
		{
			kerror(ERR_MEDERR, "       -> Caused a stack overflow and is being dealt with", pid);
		}
	
		if(regs.ebp != 0) {
			stack_trace(15, (uint32_t *)regs.ebp, iregs.eip, procs[p].symbols);
		}

		if(page_present(regs.esp)) {
			kerror(ERR_MEDERR, "      -> Stack contents:");
			uint32_t *stack = (uint32_t *)regs.esp;
			for(int i = -4; i < 8; i++) {
				if(i == -4 || i == 0 || i == 4) {
					kprintf("\n<%8X(%d)>: ", &stack[i], i);
				}
				kprintf("[%08X] ", stack[i]);
			}
		}

		exit(1);
	}

	if(regs.ebp != 0) { stack_trace(5, (uint32_t *)regs.ebp, iregs.eip, NULL); }

	kpanic("Page fault, multitasking not enabled, nothing to do to fix this.");

	for(;;);
}

static char *gpf_table_names[] = { "GDT", "IDT", "LDT", "IDT" };

void handle_gpf(uint32_t errcode, struct pusha_regs regs, struct iret_regs iregs) {
	kerror(ERR_MEDERR, "<===============================[GPF]==============================>");
	kerror(ERR_MEDERR, "General Protection Fault at 0x%08X, code seg selector %02X", iregs.eip, iregs.cs);
	kerror(ERR_MEDERR, "  -> Error code: 0x%08X", errcode);
	kerror(ERR_MEDERR, "      -> (%s) Table: %s, Sel: %04X, ",
		((errcode & 0x00000001) ? "External" : "Internal"),
		gpf_table_names[(errcode >> 1) & 0b11],
		(errcode >> 3) & 0b1111111111111
	);
	
	dump_iregs(iregs);
	dump_regs(regs);

	if(tasking) {
		int pid = current_pid;
		int p = proc_by_pid(pid);
		if(p == -1) {
			kerror(ERR_MEDERR, "  -> Failed to get process index from pid (%d)", pid);
			kpanic("GPF, halting");
			for(;;);
		}

		kerror(ERR_MEDERR, "  -> Caused by process %d [%s]", pid, procs[p].name);
	
		if(regs.ebp != 0) {
			stack_trace(15, (uint32_t *)regs.ebp, iregs.eip, procs[p].symbols);
		}
		
		exit(1);
	}

	kpanic("GPF, halting");
	for(;;);
}

void handle_invalid_op(struct pusha_regs regs, struct iret_regs iregs) {
	kerror(ERR_MEDERR, "<==============[Invalid Opcode Exception]====================>");

	dump_iregs(iregs);
	dump_regs(regs);

	kpanic("INVOP, halting");
	for(;;);
}

void handle_double_fault(struct pusha_regs regs, uint32_t errcode, struct iret_regs iregs) {
	kerror(ERR_MEDERR, "<====================[Double Fault]==========================>");

	(void)errcode;

	dump_iregs(iregs);
	dump_regs(regs);

	kpanic("DF, halting");
	for(;;);
}

void stub_error()
{
	kerror(ERR_MEDERR, "stub_error() has been called");

	//for(;;);
}


static void dump_regs(struct pusha_regs regs) {
	kerror(ERR_MEDERR, "  -> EAX: %08X EBX: %08X ECX: %08X EDX: %08X",
		regs.eax, regs.ebx, regs.ecx, regs.edx);
	kerror(ERR_MEDERR, "  -> ESP: %08X EBP: %08X EDI: %08X ESI: %08X",
		regs.esp, regs.ebp, regs.edi, regs.esi);
}

static void dump_iregs(struct iret_regs iregs) {
	kerror(ERR_MEDERR, "  -> EIP: %08X CS: %02X EFLAGS: %08X",
		iregs.eip, iregs.cs, iregs.eflags);
	kerror(ERR_MEDERR, "  -> ESP: %08X DS: %02X",
		iregs.esp, iregs.ds);
}