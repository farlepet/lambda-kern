global handle_int
global load_idt
global dummy_int


idtr DW 0 ; For limit storage
	 DD 0 ; For base storage

int_func DD 0 ; Temporary storage

load_idt:
	mov  eax, [esp + 4]
	mov  [idtr + 2], eax
	mov  ax, [esp + 8]
	mov  [idtr], ax 
	lidt [idtr]        ; Load the IDT pointer.
	ret


dummy_int:
	cli
	pusha
	mov al, 0x20
	mov dx, 0x20
	; We still must allow other interrupts to occur when we finish
	out dx, al
	popa
	sti
	iret