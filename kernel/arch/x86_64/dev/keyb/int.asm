extern handle_int
extern keyb_handle
global keyb_int

; Stub for keyboard interrupt
keyb_int:
	cli
	; Do stuff here
	sti
	iret

