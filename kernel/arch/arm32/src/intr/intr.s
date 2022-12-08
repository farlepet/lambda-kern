
.macro __INTR_BEGIN intn
    sub  lr, lr, #4
    push {lr}
    push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
    mrs  r0, spsr
    push {r0}
    sub  sp, sp, #4 /* Padding to 8-byte align stack */

    mov  r0, \intn  /* ARG0: Interrupt number */
    mov  r1, sp     /* ARG1: Pointer to data just pushed to the stack */
.endm

.macro __INTR_END
    add sp, sp, #4 /* Pop padding byte */
    pop {r0}
    msr spsr, r0
    pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
    ldm sp!, {pc}^
.endm

.global __int_wrap_undefined
.global __int_wrap_syscall
.global __int_wrap_prefetchabort
.global __int_wrap_dataabort
.global __int_wrap_hyptrap
.global __int_wrap_irq
.global __int_wrap_fiq

.extern intr_handler
.extern irq_stack_end
.extern fiq_stack_end

__int_wrap_undefined:
    __INTR_BEGIN #1
    bl intr_handler
    __INTR_END

__int_wrap_syscall:
    push {lr}
    push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
    mrs  r0, cpsr
    push {r0}
    sub  sp, sp, #4 /* Padding to 8-byte align stack */

    mov  r0, #2 /* ARG0: Interrupt Number */
    mov  r1, sp /* ARG1: Pointer to data just pushed to the stack */

    bl intr_handler

    add sp, sp, #4 /* Pop padding byte */
    pop {r0}
    msr cpsr, r0
    pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
    ldm sp!, {pc}^

__int_wrap_prefetchabort:
    __INTR_BEGIN #3
    bl intr_handler
    __INTR_END

__int_wrap_dataabort:
    __INTR_BEGIN #4
    bl intr_handler
    __INTR_END

__int_wrap_hyptrap:
    __INTR_BEGIN #5
    bl intr_handler
    __INTR_END

__int_wrap_irq:
    __INTR_BEGIN #6
    bl intr_handler
    __INTR_END

__int_wrap_fiq:
    ldr sp, =fiq_stack_end
    __INTR_BEGIN #7
    bl intr_handler
    __INTR_END


.global get_pc
# uint32_t get_pc(void)
get_pc:
    mov r0, lr
    bx  lr
