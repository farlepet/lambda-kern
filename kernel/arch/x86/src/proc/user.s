/* Enter a specified ring
 * Stack arguments (first is top of stack):
 *   - Ring to enter (0-3)
 *   - A.longress to jump to
 */
.global enter_ring
enter_ring:
    movl 4(%esp), %ebx # Ring to enter
    movl 8(%esp), %ecx # A.longress to jump to
    # mov ecx, eax

    # Get desired segment selector:
    andl $0x03, %ebx
    movl data_selectors(,%ebx,4), %eax

    # Set selectors:
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %edx # Pointer to [argc, argv, envp]

    movl %esp, %edx
    pushl %eax # Data segment
    pushl %edx # Stack pointer
    
    pushf # EFLAGS

    movl code_selectors(,%ebx,4), %eax #0x18 # User code segment
    pushl %eax # CS

    pushl %ecx # A.longress to jump to
    
    # Stack before iret:
    #  EIP
    #  CS
    #  EFLAGS
    #  ESP
    #  DS
    iret

    # SYSCALL_EXIT
    movl $3, %eax
    movl $exit_data, %ebx
    int $0xFF

.loop: jmp .loop


/* Enter a specified ring w/ new stack pointer
 * Stack arguments (first is top of stack):
 *   - Ring to enter (0-3)
 *   - Address to jump to
 *   - new esp
 */
.global enter_ring_newstack
enter_ring_newstack:
    movl %esp, %ebp

    movl 4(%ebp), %ebx # Ring to enter
    movl 8(%ebp), %ecx # Address to jump to

    # Get desired segment selector:
    andl $0x03, %ebx
    movl data_selectors(,%ebx,4), %eax

    # Set selectors:
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs


    mov 12(%ebp), %edx
    
    pushl %eax # Data segment
    pushl %edx # Stack pointer
    
    pushf # EFLAGS

    movl code_selectors(,%ebx,4), %eax #0x18 # User code segment
    pushl %eax # CS

    pushl %ecx # Address to jump to
    
    # Stack before iret:
    #  EIP
    #  CS
    #  EFLAGS
    #  ESP
    #  DS
    iret

    # SYSCALL_EXIT
    movl $3, %eax
    movl $exit_data, %ebx
    int $0xFF

enter_ring_newstack.loop: jmp .loop

.global return_from_fork
return_from_fork:
    popa
    iret


exit_data:
    .long 0x01


code_selectors:
    .long 0x08
    .long 0x19
    .long 0x2A
    .long 0x3B

data_selectors:
    .long 0x10
    .long 0x21
    .long 0x32
    .long 0x43
