extern keyb_handle
global keyb_int

; Stub for keyboard interrupt
keyb_int:
	cli
	; Do stuff here
	pusha

	; Clear eax and get keycode
	xor eax, eax
	in al, 0x60

	; Call C handler
	push eax
	call keyb_handle
	add esp, 4

	; Notify the PIC we are done
	mov al, 0x20
	out 0x20, al

	popa
	iret

