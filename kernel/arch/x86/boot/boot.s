.section .mboot
.global mboot

mboot:
	.long 0xE85250D6                             # Magic
	.long 0                                      # Archetecture
	.long mbootEnd - mboot                       # Length
	.long -(0xE85250D6 + 0 + (mbootEnd - mboot)) # Checksum
	
	.quad 0 # No idea why this is required for it to boot, padding maybe?
mbootEnd:


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
