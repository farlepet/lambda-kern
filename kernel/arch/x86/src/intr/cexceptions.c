#include <arch/proc/stack_trace.h>
#include <arch/mm/paging.h>
#include <arch/intr/int.h>

#include <proc/mtask.h>
#include <err/error.h>
#include <err/panic.h>
#include <types.h>
#include <io/output.h>
#include <mm/mm.h>

struct exception_handler {
    /** Name of exception */
    char *name;
    /** Exception handler function, if applicable. If return value is non-zero, halt or exit. */
    int (*handler)(x86_pusha_regs_t *regs, uint32_t errcode, x86_iret_regs_t *iregs);
    /** Whether exception generates errcode (for unhandled exceptions) */
    int has_errcode;
    /** Whether or not to terminate task (or halt), if there is no exception handler */
    int kill;
};

static int  _handle_page_fault(x86_pusha_regs_t *regs, uint32_t errcode, x86_iret_regs_t *iregs);
static int  _handle_gpf       (x86_pusha_regs_t *regs, uint32_t errcode, x86_iret_regs_t *iregs);

void handle_exception(uint8_t exception, x86_pusha_regs_t regs, uint32_t errcode, x86_iret_regs_t iregs);

static const struct exception_handler _exception_handlers[32] = {
    { .name = "Divide by Zero",                .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Debug",                         .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "Non Maskable Interrupt",        .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "Breakpoint",                    .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "Overflow",                      .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Bound Range Exceeded",          .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Invalid Opcode",                .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Device Not Available",          .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Double Fault",                  .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "Coprocessor Segment Overrun",   .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Invalid TSS",                   .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "Segment Not Present",           .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "Stack Segment Fault",           .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "General Protection Fault",      .handler = _handle_gpf,        .has_errcode = 1, .kill = 1 },
    { .name = "Page Fault",                    .handler = _handle_page_fault, .has_errcode = 1, .kill = 1 },
    { .name = "RESERVED[0F]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "x87 Floating Point Exception",  .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Alignment Check",               .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "Machine Check",                 .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "SIMD Floating Point Exception", .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "Virtualization Exception",      .handler = NULL,               .has_errcode = 0, .kill = 1 },
    { .name = "RESERVED[15]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[16]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[17]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[18]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[19]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[1A]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[1B]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[1C]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "RESERVED[1D]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
    { .name = "Security Exception",            .handler = NULL,               .has_errcode = 1, .kill = 1 },
    { .name = "RESERVED[1F]",                  .handler = NULL,               .has_errcode = 0, .kill = 0 },
};

static void _dump_regs(x86_pusha_regs_t *regs);
static void _dump_iregs(x86_iret_regs_t *iregs);

static void _do_stack_trace(x86_pusha_regs_t *pusha, x86_iret_regs_t *iret) {
    kproc_t   *proc   = mtask_get_curr_process();
    kthread_t *thread = mtask_get_curr_thread();
    
    if(pusha->ebp != 0) {
        symbol_t *syms = (proc != NULL) ? proc->symbols : NULL;
        stack_trace(15, (uint32_t *)pusha->ebp, iret->eip, syms);
    }
    if(proc && thread && (proc->domain == PROC_DOMAIN_USERSPACE)) {
        x86_iret_regs_t  *uiret  = (x86_iret_regs_t  *)(thread->arch.stack_kern.begin - sizeof(x86_iret_regs_t));
        if(uiret != iret) {
            /* Print userspace stack prior to kernel entry */
            x86_pusha_regs_t *upusha = (x86_pusha_regs_t *)((uintptr_t)uiret - sizeof(x86_pusha_regs_t));
            stack_trace(15, (void *)upusha->ebp, iret->eip, proc->symbols);
        }
    }

    if(mm_check_addr((void *)pusha->esp)) {
        kerror(ERR_WARN, "Stack contents:");
        uint32_t *stack = (uint32_t *)pusha->esp;
        for(int i = -4; i < 8; i++) {
            if(!mm_check_addr(&stack[i])) {
                break;
            }
            if(i == -4 || i == 0 || i == 4) {
                if(i != -4) {
                    kput('\n');
                }
                kprintf("<%8X(% d)>: ", &stack[i], i);
            }
            kprintf("[%08X] ", stack[i]);
        }
        kput('\n');
    }
}

/**
 * C side of page fault handler.
 *
 * @param errcode errorcode pushed on stack by the fault
 * @param cr3 value of cr3 register (location of fault)
 */
static int _handle_page_fault(x86_pusha_regs_t *regs, uint32_t errcode, x86_iret_regs_t *iregs) {
    kthread_t *curr_thread = mtask_get_curr_thread();
    kproc_t   *curr_proc   = mtask_get_curr_process();
    
    uint32_t *cr3 = get_pagedir();

    uint32_t cr2;
    asm volatile("movl %%cr2, %0": "=a"(cr2));

    /* @todo Don't print anything if fault is handled w/o killing the task */

    kerror(ERR_WARN, "Page fault at 0x%08X --> 0x%08X (%s%s%s%s%s)", cr2, pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFFFF000,
                ((errcode & 0x01) ? "present"                   : "non-present"),
                ((errcode & 0x02) ? ", write"                   : ", read"),
                ((errcode & 0x04) ? ", user-mode"               : ", kernel-mode"),
                ((errcode & 0x08) ? ", modified reserved field" : ""),
                ((errcode & 0x10) ? ", instruction fetch"       : ""));

    _dump_iregs(iregs);
    _dump_regs(regs);

    kerror(ERR_WARN, "  -> Page flags:  0x%03X", pgdir_get_page_entry(cr3, (void *)cr2) & 0xFFF);
    kerror(ERR_WARN, "  -> Table flags: 0x%03X", pgdir_get_page_table(cr3, (void *)cr2) & 0xFFF);
    kerror(ERR_WARN, "  -> Page Directory: 0x%08X", cr3);
    kerror(ERR_WARN, "  -> Kernel pagedir: 0x%08X", kernel_cr3);


    if(curr_proc && mm_check_addr(curr_proc)) {
        kerror(ERR_WARN, "  -> Caused by process %d [%s]", curr_proc->pid, curr_proc->name);
        kerror(ERR_WARN, "      -> Thread %d [%s]", curr_thread->tid, curr_thread->name);
        kerror(ERR_WARN, "      -> Entrypoint: %08X", curr_thread->entrypoint);

        if(((cr2 > curr_thread->arch.stack_user.begin) &&
            (cr2 < (curr_thread->arch.stack_user.begin + 0x1000))) ||
           ((cr2 < (curr_thread->arch.stack_user.begin - curr_thread->arch.stack_user.size)) &&
            ((cr2 > (curr_thread->arch.stack_user.begin - curr_thread->arch.stack_user.size - 0x1000))))) {
            kerror(ERR_WARN, "       -> Caused a stack overflow and is being dealt with");
        }

        return 1;
    } else if (curr_proc && !mm_check_addr(curr_proc)) {
        kerror(ERR_WARN, "  -> curr_proc is corrupted: %p", curr_proc);
    }

    if(cr2 >= KERNEL_OFFSET && !(errcode & 0x01)) {
        /* Exception in shared kernel pages */
        uint32_t didx = cr2 >> 22;
        
        uint32_t *pgdir  = (uint32_t *)cr3;
        uint32_t *kpgdir = (uint32_t *)kernel_cr3;
        if(!(pgdir[didx] & PAGE_DIRECTORY_FLAG_PRESENT) &&
           (kpgdir[didx] & PAGE_DIRECTORY_FLAG_PRESENT)) {
            kerror(ERR_WARN, "  -> Mapping kernel page");
            pgdir[didx] = kpgdir[didx];
            return 0;
        } else {
            kerror(ERR_WARN, "  -> In kernel range, but not a missing mapping");
        }
    }

    return 1;
}

static const char *gpf_table_names[] = { "GDT", "IDT", "LDT", "IDT" };

static int _handle_gpf(x86_pusha_regs_t *regs, uint32_t errcode, x86_iret_regs_t *iregs) {
    kproc_t *curr_proc  = mtask_get_curr_process();

    kerror(ERR_WARN, "<===============================[GPF]==============================>");
    kerror(ERR_WARN, "General Protection Fault at 0x%08X, code seg selector %02X", iregs->eip, iregs->cs);
    kerror(ERR_WARN, "  -> Error code: 0x%08X", errcode);
    kerror(ERR_WARN, "      -> (%s) Table: %s, Sel: %04X, ",
        ((errcode & 0x00000001) ? "External" : "Internal"),
        gpf_table_names[(errcode >> 1) & 0b11],
        (errcode >> 3) & 0b1111111111111
    );

    _dump_iregs(iregs);
    _dump_regs(regs);

    if(curr_proc) {
        kerror(ERR_WARN, "  -> Caused by process %d [%s]", curr_proc->pid, curr_proc->name);
    }

    return 1;
}


void handle_exception(uint8_t exception, x86_pusha_regs_t regs, uint32_t errcode, x86_iret_regs_t iregs) {
    kproc_t *curr_proc = mtask_get_curr_process();
    
    if(exception < (sizeof(_exception_handlers) / sizeof(_exception_handlers[0]))) {
        const struct exception_handler *hand = &_exception_handlers[exception];
        kerror(ERR_WARN, "EXCEPTION OCCURED: %s", hand->name);

        int do_kill = 0;

        if(hand->handler) {
            do_kill = hand->handler(&regs, errcode, &iregs);
        } else {
            if(hand->has_errcode) {
                kerror(ERR_WARN, "  -> errcode: %08X", errcode);
            }
            _dump_iregs(&iregs);
            _dump_regs(&regs);

            do_kill = hand->kill;
        }

        _do_stack_trace(&regs, &iregs);

        if(do_kill) {
            if(curr_proc && mm_check_addr(curr_proc)) {
                kerror(ERR_WARN, "Killing task");
                exit(1);
            }

            kpanic("Multitasking not enabled - halting");
        }

        return;
    }
    
    kpanic("Unhandled exception ID: 0x%02X", exception);
}

static void _dump_regs(x86_pusha_regs_t *regs) {
    kerror(ERR_WARN, "  -> EAX: %08X EBX: %08X ECX: %08X EDX: %08X",
        regs->eax, regs->ebx, regs->ecx, regs->edx);
    kerror(ERR_WARN, "  -> ESP: %08X EBP: %08X EDI: %08X ESI: %08X",
        regs->esp, regs->ebp, regs->edi, regs->esi);
}

static void _dump_iregs(x86_iret_regs_t *iregs) {
    kerror(ERR_WARN, "  -> EIP: %08X CS: %02X EFLAGS: %08X",
        iregs->eip, iregs->cs, iregs->eflags);
    kerror(ERR_WARN, "  -> ESP: %08X DS: %02X",
        iregs->esp, iregs->ds);
}

