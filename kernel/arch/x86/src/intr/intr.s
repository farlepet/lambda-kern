.global load_idt
.global dummy_int

.extern stub_error

# Set the interrupt descriptor table
load_idt:
	movl 4(%esp), %eax
	movl %eax, (idtr + 2)
	movw 8(%esp), %ax
	movw %ax, (idtr) 
	lidt (idtr)        # Load the IDT pointer.
	ret

# Dummy interrupt, so all interrupts can be enables, with no immediate problems
dummy_int:
	cli
	pusha
	call stub_error
	movb $0x20, %al
	outb %al, $0x20
	outb %al, $0xA0

	popa
	iret




.global pit_int
.extern pit_handler
.extern do_task_switch
# Interrupt handler called by the PIT
pit_int:
	cli
	pusha

	call pit_handler

	#call do_task_switch

	popa
	iret


.global sched_run
# This is an interrupt that forces the scheduler to run
sched_run:
	cli
	pusha

	call do_task_switch

	popa
	iret


.global interrupts_enabled
# Checks to see if interrupts are currently enabled
interrupts_enabled:
	pushf
	popl %eax
	andl $(1 << 9), %eax # Get IF flag
	shrl $9, %eax
	ret







.extern handle_syscall
.global syscall_int
# Interrupt used when issuing a syscall
syscall_int:
	pusha

	#push ebx ; Arguments pointer
	#push eax ; Syscall number
	#push esp

	call handle_syscall

	#add esp, 8
	#add esp, 4

	popa
	iret

.extern restore_syscall_regs
.global return_from_syscall
return_from_syscall:
	popa
	iret

.global call_syscall_int
# Call interrupt for a syscall
call_syscall_int:
	movl %esp, %ebp
	pusha

	movl 4(%ebp), %eax
	movl 8(%ebp), %ebx

	int $0xFF

	popa
	ret

.section .data
idtr: .word 0 # For limit storage
	  .long 0 # For base storage
