.section .entryp

.extern __int_wrap_undefined
.extern __int_wrap_syscall
.extern __int_wrap_prefetchabort
.extern __int_wrap_dataabort
.extern __int_wrap_hyptrap
.extern __int_wrap_irq
.extern __int_wrap_fiq

.global __int_table
__int_table:
    b reset
    b __int_wrap_undefined
    b __int_wrap_syscall
    b __int_wrap_prefetchabort
    b __int_wrap_dataabort
    b __int_wrap_hyptrap
    b __int_wrap_irq
    b __int_wrap_fiq

.extern kentry
.extern new_stack_end
.extern irq_stack_end
.extern fiq_stack_end
.global reset
.type reset, %function
reset:
    /* Stack initialization for various modes */
    /* FIQ: */
    msr cpsr_c, 0x11 /* FIQ mode */
    ldr sp, =fiq_stack_end

    msr cpsr_c, 0x12 /* IRQ mode */
    ldr sp, =irq_stack_end

    msr cpsr_c, 0x13 /* SVC mode */
    ldr sp, =new_stack_end

    /*ldr r0, =new_stack_end
    mov sp, r0*/
    bl kentry

endloop:
    wfi
    b endloop

.size reset, . - reset
