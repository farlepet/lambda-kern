.set STACK_SZ, 0x2000

.global start
.extern kentry
.type start, @function
start:
	mov $(new_stack_t + STACK_SZ), %esp
    mov %esp, %ebp

	push %eax # Bootloader magic number
	push %ebx # Bootloader data header

	cli
	call kentry
	cli

endloop:
	hlt
	jmp endloop

.size start, . - start


.global get_eip
get_eip:
	mov (%esp), %eax
	ret


.lcomm new_stack_t, STACK_SZ
