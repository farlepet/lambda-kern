.section .entryp

.extern kentry
.global reset
.type reset, %function
reset:
    # Shutdown all cores but one:
    mrc p15, 0, r5, c0, c0, 5
    and r5, r5, #3
    cmp r5, #0
    bne endloop

    ldr r5, =reset
    mov sp, r5

jump_to_entry:
    bl kentry

endloop:
    wfi
    b endloop

.size reset, . - reset

.global __int_table
__int_table:
    b reset
    b reset
    b reset
    b reset
    b reset
    b reset
    b reset
    b reset
