#include <proc/mtask.h>
#include <intr/intr.h>
#include <err/panic.h>
#include <mm/mm.h>
#include <types.h>
#include <video.h>

/* TODO: Create unified stacktrace interface */
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
#  include <arch/proc/stack_trace.h>
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARM32)
#  include <arch/proc/stacktrace.h>
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
    if(thread && mm_check_addr(thread)) {
        kproc_t *proc = thread->process;
        kprintf("    Thread  %d [%s]\n", thread->tid, thread->name);
        if(proc && mm_check_addr(proc)) {
            kprintf("    Process %d [%s]\n", proc->pid,   proc->name);
        }
    }

    // Print regs here
#if (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_X86)
    stack_trace(16, __builtin_frame_address(0), (uint32_t)&stack_trace_here, NULL);
#elif (__LAMBDA_PLATFORM_ARCH__ == PLATFORM_ARCH_ARM32)
    stacktrace_here(16);
#endif
    
    arch_kpanic_hook();

    for(;;) interrupt_halt();
}
