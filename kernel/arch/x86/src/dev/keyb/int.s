.extern keyb_handle
.global keyb_int

# Stub for keyboard interrupt
keyb_int:
	cli
	# Do stuff here
	pusha

	# Clear eax and get keycode
	xorl %eax, %eax
	inb $0x60, %al

	# Call C handler
	pushl %eax
	call keyb_handle
	addl $4, %esp

	# Notify the PIC we are done
	movb $0x20, %al
	outb %al, $0x20

	popa
	iret

