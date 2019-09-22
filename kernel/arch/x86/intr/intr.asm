global handle_int
global load_idt
global dummy_int

extern stub_error


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
	call stub_error
	mov al, 0x20
	out 0x20, al
	out 0xA0, al

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


global sched_run
; This is an interrupt that forces the scheduler to run
sched_run:
	cli
	pusha

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







extern handle_syscall
global syscall_int
; Interrupt used when issuing a syscall
syscall_int:
	pusha

	;push ebx ; Arguments pointer
	;push eax ; Syscall number
	push esp

	call handle_syscall

	;add esp, 8
	add esp, 4

	popa
	iret

extern restore_syscall_regs
global return_from_syscall
return_from_syscall:
	popa
	iret

global call_syscall_int
; Call interrupt for a syscall
call_syscall_int:
	mov ebp, esp
	pusha

	mov eax, [ebp + 4]
	mov ebx, [ebp + 8]

	int 0xFF

	popa
	ret
