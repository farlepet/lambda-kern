#include <proc/mtask.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <mm/mm.h>
#include <types.h>
#include <io/output.h>

__weak
void arch_kpanic_hook(void) {}

__noreturn
void _kpanic(char *msg, ...)
{
    disable_interrupts();
    
    __builtin_va_list varg;
    __builtin_va_start(varg, msg);
    
    kprintf("Kernel panic:\n    ");
    kprintv(msg, varg);
    kput('\n');
    
    __builtin_va_end(varg);
    
    /* TODO: Multiprocessor support */
    kthread_t *thread = mtask_get_curr_thread();
    if(thread && mm_check_addr(thread)) {
        kproc_t *proc = thread->process;
        kprintf("    Thread  %d [%s]\n", thread->tid, thread->name);
        if(proc && mm_check_addr(proc)) {
            kprintf("    Process %d [%s]\n", proc->pid,   proc->name);
        }
    }

    arch_kpanic_hook();

    for(;;) interrupt_halt();
}
