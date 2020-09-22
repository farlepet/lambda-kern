.section .entryp
.extern kmain
.extern new_stack_end
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
