#include <proc/mtask.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <types.h>
#include <video.h>

#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/proc/stack_trace.h>
#endif

__weak
void arch_kpanic_hook(void) {

}

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
    if(thread) {
        kproc_t *proc = thread->process;
        kprintf("    Thread  %d [%s]\n", thread->tid, thread->name);
        kprintf("    Process %d [%s]\n", proc->pid,   proc->name);
    }

    // Print regs here
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    stack_trace(16, __builtin_frame_address(0), (uint32_t)&stack_trace_here, NULL);
#endif
    
    arch_kpanic_hook();

    for(;;) interrupt_halt();
}
