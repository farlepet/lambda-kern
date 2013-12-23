[BITS 64]

global dummy_int
global dummy_errcode

dummy_int:
	pushaq
	mov al, 0x20
	out 0x20, al
	out 0xA0, al
	popaq
	iret

dummy_errcode:
	pushaq
	pop rax ; errorcode
	popaq
	iret