#include <arch/proc/stack_trace.h>
#include <arch/mm/paging.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <types.h>
#include <video.h>
#include <mm/mm.h>

struct exception_handler {
	/** Name of exception */
	char *name;
	/** Exception handler function, if applicable. If return value is non-zero, halt or exit. */
	int (*handler)(struct pusha_regs *regs, uint32_t errcode, struct iret_regs *iregs);
	/** Whether exception generates errcode (for unhandled exceptions) */
	int has_errcode;
	/** Whether or not to terminate task (or halt), if there is no exception handler */
	int kill;
};

int handle_page_fault(struct pusha_regs *regs, uint32_t errcode, struct iret_regs *iregs);
int handle_gpf(struct pusha_regs *regs, uint32_t errcode, struct iret_regs *iregs);
void handle_exception(uint8_t exception, struct pusha_regs regs, uint32_t errcode, struct iret_regs iregs);
void stub_error(void);

static const struct exception_handler exception_handlers[32] = {
	{ .name = "Divide by Zero",                .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Debug",                         .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "Non Maskable Interrupt",        .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "Breakpoint",                    .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "Overflow",                      .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Bound Range Exceeded",          .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Invalid Opcode",                .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Device Not Available",          .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Double Fault",                  .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "Coprocessor Segment Overrun",   .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Invalid TSS",                   .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "Segment Not Present",           .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "Stack Segment Fault",           .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "General Protection Fault",      .handler = handle_gpf       , .has_errcode = 1, .kill = 1 },
	{ .name = "Page Fault",                    .handler = handle_page_fault, .has_errcode = 1, .kill = 1 },
	{ .name = "RESERVED[0F]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "x87 Floating Point Exception",  .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Alignment Check",               .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "Machine Check",                 .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "SIMD Floating Point Exception", .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "Virtualization Exception",      .handler = NULL,              .has_errcode = 0, .kill = 1 },
	{ .name = "RESERVED[15]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[16]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[17]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[18]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[19]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[1A]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[1B]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[1C]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "RESERVED[1D]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
	{ .name = "Security Exception",            .handler = NULL,              .has_errcode = 1, .kill = 1 },
	{ .name = "RESERVED[1F]",                  .handler = NULL,              .has_errcode = 0, .kill = 0 },
};

static void dump_regs(struct pusha_regs *regs);
static void dump_iregs(struct iret_regs *iregs);

/**
 * C side of page fault handler.
 *
 * @param errcode errorcode pushed on stack by the fault
 * @param cr3 value of cr3 register (location of fault)
 */
int handle_page_fault(struct pusha_regs *regs, uint32_t errcode, struct iret_regs *iregs) {
    kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = curr_thread->process;
	
	uint32_t *cr3 = get_pagedir();

	uint32_t cr2;
	asm volatile("movl %%cr2, %0": "=a"(cr2));

	kerror(ERR_WARN, "Page fault at 0x%08X --> 0x%08X (%s%s%s%s%s)", cr2, pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFFFF000,
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
		kerror(ERR_WARN, "  -> On frame %08X(%d)", frame, frame);
	}
	else kerror(ERR_WARN, "  -> Occurred in kernel-space, not in the page frames");

	kerror(ERR_WARN, "  -> Page flags:  0x%03X", pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFF);
	kerror(ERR_WARN, "  -> Table flags: 0x%03X", pgdir_get_page_table(cr3, (void *)cr2) & 0xFFF);
	kerror(ERR_WARN, "  -> Page Directory: 0x%08X", cr3);
	kerror(ERR_WARN, "  -> Kernel pagedir: 0x%08X", kernel_cr3);


	if(curr_proc) {
		kerror(ERR_WARN, "  -> Caused by process %d [%s]", curr_proc->pid, curr_proc->name);
		kerror(ERR_WARN, "      -> Thread %d [%s]", curr_thread->tid, curr_thread->name);
		kerror(ERR_WARN, "      -> Entrypoint: %08X", curr_thread->entrypoint);

		if(((cr2 > curr_thread->arch.stack_user.begin) &&
		    (cr2 < (curr_thread->arch.stack_user.begin + 0x1000))) ||
		   ((cr2 < (curr_thread->arch.stack_user.begin - curr_thread->arch.stack_user.size)) &&
		    ((cr2 > (curr_thread->arch.stack_user.begin - curr_thread->arch.stack_user.size - 0x1000))))) {
			kerror(ERR_WARN, "       -> Caused a stack overflow and is being dealt with");
		}
	
		if(regs->ebp != 0) {
			stack_trace(15, (uint32_t *)regs->ebp, iregs->eip, curr_proc->symbols);
		}

		if(page_present(regs->esp)) {
			kerror(ERR_WARN, "      -> Stack contents:");
			uint32_t *stack = (uint32_t *)regs->esp;
			for(int i = -4; i < 8; i++) {
				if(!mm_check_addr(&stack[i])) {
					break;
				}
				if(i == -4 || i == 0 || i == 4) {
					if(i != -4) {
						kput('\n');
					}
					kprintf("<%8X(%d)>: ", &stack[i], i);
				}
				kprintf("[%08X] ", stack[i]);
			}
			kput('\n');
		}

		/* Halt */
		return 1;
	}

	if(regs->ebp != 0) { stack_trace(5, (uint32_t *)regs->ebp, iregs->eip, NULL); }

	/* Halt */
	return 1;
}

static char *gpf_table_names[] = { "GDT", "IDT", "LDT", "IDT" };

int handle_gpf(struct pusha_regs *regs, uint32_t errcode, struct iret_regs *iregs) {
    kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = curr_thread->process;
	
	kerror(ERR_WARN, "<===============================[GPF]==============================>");
	kerror(ERR_WARN, "General Protection Fault at 0x%08X, code seg selector %02X", iregs->eip, iregs->cs);
	kerror(ERR_WARN, "  -> Error code: 0x%08X", errcode);
	kerror(ERR_WARN, "      -> (%s) Table: %s, Sel: %04X, ",
		((errcode & 0x00000001) ? "External" : "Internal"),
		gpf_table_names[(errcode >> 1) & 0b11],
		(errcode >> 3) & 0b1111111111111
	);
	
	dump_iregs(iregs);
	dump_regs(regs);

	if(curr_proc) {
		kerror(ERR_WARN, "  -> Caused by process %d [%s]", curr_proc->pid, curr_proc->name);
	
		if(regs->ebp != 0) {
			stack_trace(15, (uint32_t *)regs->ebp, iregs->eip, curr_proc->symbols);
		}
	}

	/* Halt */
	return 1;
}



void handle_exception(uint8_t exception, struct pusha_regs regs, uint32_t errcode, struct iret_regs iregs) {
    kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = curr_thread->process;
	
	if(exception < (sizeof(exception_handlers) / sizeof(exception_handlers[0]))) {
		const struct exception_handler *hand = &exception_handlers[exception];
		kerror(ERR_WARN, "EXCEPTION OCCURED: %s", hand->name);

		if(hand->handler) {
			if(hand->handler(&regs, errcode, &iregs)) {
				if(curr_proc) {
					kerror(ERR_WARN, "Killing task");
					exit(1);
				}
				
				kpanic("Halting");
			}
		} else {
			if(hand->has_errcode) {
				kerror(ERR_WARN, "  -> errcode: %08X", errcode);
			}
			dump_iregs(&iregs);
			dump_regs(&regs);

			if(hand->kill) {
				if(curr_proc) {
					kerror(ERR_WARN, "Killing task");
					exit(1);
				}
				
				kpanic("Halting");
			}
		}

		return;
	}
	
	kpanic("Unhandled exception ID: 0x%02X", exception);
}


void stub_error()
{
	kerror(ERR_WARN, "stub_error() has been called");

	//for(;;);
}


static void dump_regs(struct pusha_regs *regs) {
	kerror(ERR_WARN, "  -> EAX: %08X EBX: %08X ECX: %08X EDX: %08X",
		regs->eax, regs->ebx, regs->ecx, regs->edx);
	kerror(ERR_WARN, "  -> ESP: %08X EBP: %08X EDI: %08X ESI: %08X",
		regs->esp, regs->ebp, regs->edi, regs->esi);
}

static void dump_iregs(struct iret_regs *iregs) {
	kerror(ERR_WARN, "  -> EIP: %08X CS: %02X EFLAGS: %08X",
		iregs->eip, iregs->cs, iregs->eflags);
	kerror(ERR_WARN, "  -> ESP: %08X DS: %02X",
		iregs->esp, iregs->ds);
}
