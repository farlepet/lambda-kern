.section .mboot
.global mboot

.align 8
mboot:
	.long 0xE85250D6                             # Magic
	.long 0                                      # Archetecture
	.long mbootEnd - mboot                       # Length
	.long -(0xE85250D6 + 0 + (mbootEnd - mboot)) # Checksum
	
# Muiltboot Information Request
  .mbi_start:
	.word 1
	.word 0
	.long .mbi_end - .mbi_start
	.long 1 # Command line tag
	.long 4 # Basic memory tag
	.long 6 # Memory map tag
  .mbi_end:

	.quad 0 # No idea why this is required for it to boot, padding maybe?
mbootEnd:


.global start
.extern kmain

start:
	pushl %eax
	pushl %ebx

	cli
	call kmain
	cli

endloop:
	hlt
	jmp endloop
