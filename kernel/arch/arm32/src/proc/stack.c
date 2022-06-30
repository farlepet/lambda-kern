#include <arch/proc/stack.h>

#include <err/error.h>
#include <err/panic.h>
#include <string.h>

int proc_copy_stack(kthread_t *dest, const kthread_t *src) {
    kpanic("proc_copy_stack() not yet implemented on this architecture!");

    (void)dest;
    (void)src;

    return 0;
}

int proc_copy_kernel_stack(kthread_t *dest, const kthread_t *src) {
    kpanic("proc_copy_kernel_stack() not yet implemented on this architecture!");

    (void)dest;
    (void)src;

    return 0;
}