#include <stdint.h>

#include <proc/syscalls.h>

__attribute__((interrupt ("SWI")))
void syscall_int(uint32_t r0) {
    __SWI_BEGIN;
    uint32_t scn;
    asm volatile("ldrb %0, [lr, #-1]": "=r" (scn));

    service_syscall(scn, (uint32_t *)r0);
    __INTR_END;
}