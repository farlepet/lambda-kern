#include <stdint.h>

#include <intr/types/intr.h>
#include <proc/mtask.h>
#include <proc/syscalls.h>

void arch_syscall_interrupt(intr_handler_hand_t *hdlr) {
    (void)hdlr;

    /* @todo */
    uint32_t      scn  = 0;
    syscallarg_t *args = NULL;

    if(syscall_service(scn, args)) {
        /* @todo Stack trace */
        exit(1);
    }
}

