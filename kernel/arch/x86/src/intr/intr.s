.altmacro

.extern idt_set_entry



.macro gen_int_handler num
int_handler_\num:
	pusha
	push $\num
	call idt_handle_interrupt

.if \num < 40
	movb $0x20, %al
	outb %al, $0x20
.elseif \num < 48
	movb $0x20, %al
	outb %al, $0xA0
.endif

	addl $4, %esp # Int number
	popa
	iret
.endm

.set n, 32
.rept (256 - 32)
    gen_int_handler %n
    .set n, n+1
.endr


.macro setidt intr, sel, flags, func
	pusha
	pushl \func
	pushl \flags
	pushl \sel
	pushl \intr
	call idt_set_entry
	addl $16, %esp
	popa
.endm

.macro setidt_intr num
    setidt $\num $0x08 $0x8E $int_handler_\num
.endm

.global idt_setup_handlers
idt_setup_handlers:
.set n, 32
.rept (256 - 32)
    setidt_intr %n
    .set n, n+1
.endr
    ret




.global interrupts_enabled
# Checks to see if interrupts are currently enabled
interrupts_enabled:
	pushf
	popl %eax
	andl $(1 << 9), %eax # Get IF flag
	shrl $9, %eax
	ret



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

