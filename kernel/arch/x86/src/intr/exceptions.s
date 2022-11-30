# Contains all exception ISR entrypoints

.altmacro

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

.extern handle_exception

.macro except_isr num err
exception_isr_\num:
.if !\err
	push $0 # Errcode
.endif
	pusha
	push $\num
	call handle_exception
	popa
.if !\err
	addl $4, %esp # Errcode
.endif
	iret
.endm

.macro setidt_exception num
    setidt $\num, $0x08, $0x8E, $exception_isr_\num
.endm

.global idt_setup_exceptions
idt_setup_exceptions:
.set n, 0
.rept 32
    setidt_exception %n
    .set n, n+1
.endr
	ret


except_isr 0,  0
except_isr 1,  0
except_isr 2,  0
except_isr 3,  0
except_isr 4,  0
except_isr 5,  0
except_isr 6,  0
except_isr 7,  0
except_isr 8,  1
except_isr 9,  0
except_isr 10, 1
except_isr 11, 1
except_isr 12, 1
except_isr 13, 1
except_isr 14, 1
except_isr 15, 0
except_isr 16, 0
except_isr 17, 1
except_isr 18, 0
except_isr 19, 0
except_isr 20, 0
except_isr 21, 0
except_isr 22, 0
except_isr 23, 0
except_isr 24, 0
except_isr 25, 0
except_isr 26, 0
except_isr 27, 0
except_isr 28, 0
except_isr 29, 0
except_isr 30, 1
except_isr 31, 0

