global handle_int
global load_idt
global dummy_int


idtr DW 0 ; For limit storage
	 DQ 0 ; For base storage

load_idt:
	mov  rax, [rsp + 4]
	mov  [QWORD idtr + 2], rax
	mov  ax, [rsp + 8]
	mov  [QWORD idtr], ax 
	mov  rax, idtr
	lidt [rax]
	ret


dummy_int:
	cli
	jmp dummy_int
	
	pushaq
	mov al, 0x20
	mov dx, 0x20
	; We still must allow other interrupts to occur when we finish
	out dx, al
	popaq
	sti
	iretq