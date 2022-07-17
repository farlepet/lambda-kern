.section .entryp

.global __int_table
__int_table:
    b reset                    /* + 0x00 */
    b __int_wrap_undefined     /* + 0x04 */
    b __int_wrap_syscall       /* + 0x08 */
    b __int_wrap_prefetchabort /* + 0x0C */
    b __int_wrap_dataabort     /* + 0x10 */
    b __int_wrap_hyptrap       /* + 0x14 */
    b __int_wrap_irq           /* + 0x18 */
    b __int_wrap_fiq           /* + 0x20 */
.size __int_table, . - __int_table

.extern new_stack_end
.extern irq_stack_end
.extern fiq_stack_end

.extern kern_premap
.global reset
.type reset, %function
reset:
    /* Shutdown all cores but one: */
    mrc p15, 0, r5, c0, c0, 5
    and r5, r5, #3
    cmp r5, #0
    bne endloop

    /* Check if we are in HYP mode */
    mrs r0, cpsr
    and r2, r0, #0x1F
    mov r1, #0x1A
    cmp r2, r1
    bne _set_stacks

_exit_hyp:
    bic r0, r0, #0x1F
    orr r0, r0, #0xD3  /* SVC, disabled interrupts */
    orr r0, r0, #0x100 /* Disabled data abort */
    msr spsr_cxsf, r0
    ldr lr, =_set_stacks
   .long 0xE12EF30E    /* "msr ELR_hyp, lr" */
   .long 0xE160006E    /* "eret" */

_set_stacks:
    msr cpsr_c, 0x11 /* FIQ mode */
    ldr sp, =fiq_stack_end
    msr cpsr_c, 0x12 /* IRQ mode */
    ldr sp, =irq_stack_end
    msr cpsr_c, 0x17 /* ABT mode */
    ldr sp, =irq_stack_end
    msr cpsr_c, 0x1B /* UND mode */
    ldr sp, =irq_stack_end
    msr cpsr_c, 0x1F /* SVC mode */
    ldr sp, =new_stack_end
    sub sp, sp, #0xC0000000

    bl kern_premap

endloop:
    wfi
    b endloop

.size reset, . - reset
