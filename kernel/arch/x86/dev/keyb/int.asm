extern keyb_handle
global keyb_int

; Stub for keyboard interrupt
keyb_int:
	cli
	; Do stuff here
	pusha

	xor eax, eax
	in al, 0x60

	push eax
	call keyb_handle
	add esp, 4

	mov al, 0x20
	out 0x20, al

	popa
	iret

