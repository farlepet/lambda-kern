extern handle_int
extern keyb_handle
global keyb_int

; Stuub for keyboard interrupt
keyb_int:
	cli
	push keyb_handle
	jmp handle_int

