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



global interrupts_enabled
interrupts_enabled:
	pushf
	pop rax
	and rax, (1 << 10) ; Get IF flag
	shl rax, 9
	ret