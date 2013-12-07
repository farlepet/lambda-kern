.section .mboot
.global mboot

mboot:
	.long 0x1BADB002           # Magic
	.long 0x03                 # Flags
	.long -(0x1BADB002 + 0x03) # Checksum

.global start
.extern kmain

start:
	pushl %esp
	pushl %ebx

	cli
	call kmain
	cli

endloop:
	hlt
	jmp endloop



.extern test_int
.global inttest
inttest:
	cli
	pushal
	call test_int
	popal
	sti
	iret
