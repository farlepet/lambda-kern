global handle_int
global load_idt
global dummy_int


idtr DW 0 ; For limit storage
	 DD 0 ; For base storage

; Set the interrupt descriptor table
load_idt:
	mov  eax, [esp + 4]
	mov  [idtr + 2], eax
	mov  ax, [esp + 8]
	mov  [idtr], ax 
	lidt [idtr]        ; Load the IDT pointer.
	ret

; Dummy interrupt, so all interrupts can be enables, with no immediate problems
dummy_int:
	cli
	pusha
	mov al, 0x20
	out 0x20, al
	out 0xA0, al

	call do_task_switch

	popa
	iret




global pit_int
extern pit_handler
extern do_task_switch
; Interrupt handler called by the PIT
pit_int:
	cli
	pusha

	call pit_handler

	call do_task_switch

	popa
	iret



global interrupts_enabled
; Checks to see if interrupts are currently enabled
interrupts_enabled:
	pushf
	pop eax
	and eax, (1 << 9) ; Get IF flag
	shr eax, 9
	ret
