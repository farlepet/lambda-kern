#include <arch/proc/tasking.h>

int arch_proc_create_stack(struct kproc *proc, size_t stack_size, uintptr_t virt_stack_begin, int is_kernel) {
    (void)proc;
    (void)stack_size;
    (void)virt_stack_begin;
    (void)is_kernel;

    /* TODO */
    return 0;
}

int arch_proc_create_kernel_stack(struct kproc *proc) {
    (void)proc;

    /* TODO */
    return 0;
}

int arch_setup_task(struct kproc *proc, void *entrypoint, uint32_t stack_size, int kernel, arch_task_params_t *arch_params) {
    (void)proc;
    (void)entrypoint;
    (void)stack_size;
    (void)kernel;
    (void)arch_params;

    /* TODO */
    return 0;
}

void arch_multitasking_init(void) {
    /* TODO */
}

__hot void do_task_switch(void) {
    /* TODO */
}