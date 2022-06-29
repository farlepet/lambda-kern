.set STACK_SZ, 0x2000

.global start
.extern kern_premap
.section .boot.text
.type start, @function
start:
	//mov $(new_stack_t + STACK_SZ), %esp
    mov %esp, %ebp

	push %eax # Bootloader magic number
	push %ebx # Bootloader data header

	cli
	call kern_premap
	cli

endloop:
	hlt
	jmp endloop

.size start, . - start


.section .text
.global get_eip
get_eip:
	mov (%esp), %eax
	ret


.section .boot.data
.lcomm new_stack_t, STACK_SZ
