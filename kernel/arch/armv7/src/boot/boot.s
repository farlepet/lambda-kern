.section .entryp
.extern kmain
.global start
.type start, %function
start:
    ldr r0, =new_stack_end
    mov sp, r0
    bl kmain

endloop:
    wfi
    b endloop

.size start, . - start

.lcomm new_stack_t, 0x10000
new_stack_end: .long 0
