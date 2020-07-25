.section .mboot
.global mboot

.align 8
mboot:
#	.long 0xE85250D6                             # Magic
#	.long 0                                      # Archetecture
#	.long mbootEnd - mboot                       # Length
#	.long -(0xE85250D6 + 0 + (mbootEnd - mboot)) # Checksum
#	
# Muiltboot Information Request
#  .mbi_start:
#	.word 1
#	.word 0
#	.long .mbi_end - .mbi_start
#	.long 1 # Command line tag
#	.long 4 # Basic memory tag
#	.long 6 # Memory map tag
#  .mbi_end:
#
#	.quad 0

	.long 0x1BADB002
	.long 0x03
	.long -(0x1BADB002 + 0x03)

mbootEnd:


.global start
.extern kmain
.type start, @function
start:
	mov $(new_stack_t + 0x10000), %esp
    mov %esp, %ebp

	pushl %eax # Bootloader magic number
	pushl %ebx # Bootloader data header

	cli
	call kmain
	cli

endloop:
	hlt
	jmp endloop

.size start, . - start


.global get_eip
get_eip:
	;pop %eax
	;push %eax
	mov (%esp), %eax
	ret


.lcomm new_stack_t, 0x10000
