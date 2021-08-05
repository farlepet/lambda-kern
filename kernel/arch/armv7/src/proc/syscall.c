#include <stdint.h>

#include <proc/syscalls.h>

__hot
__attribute__((interrupt ("SWI")))
void syscall_int(uint32_t r0) {
    uint32_t scn;
    /* This line causes an undefined instruction exception: */
    asm volatile("ldr %0, [lr, #-4]": "=r" (scn));
    scn &= 0x00FFFFFF;

    /* This is to prevent failure due to the following error:
     * call to function without interrupt attribute could clobber interruptee's VFP registers
     * TODO: Actually save VFP registers */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra"
    service_syscall(scn, (uint32_t *)r0);
#pragma clang diagnostic pop
}