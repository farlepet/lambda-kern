# Contains all exception ISR entrypoints

.macro setidt intr, sel, flags, func
	pusha
	pushl \func
	pushl \flags
	pushl \sel
	pushl \intr
	call set_idt
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

.global exceptions_init
exceptions_init:
	setidt $0x00, $0x08, $0x8E, $exception_isr_0
	setidt $0x01, $0x08, $0x8E, $exception_isr_1
	setidt $0x02, $0x08, $0x8E, $exception_isr_2
	setidt $0x03, $0x08, $0x8E, $exception_isr_3
	setidt $0x04, $0x08, $0x8E, $exception_isr_4
	setidt $0x05, $0x08, $0x8E, $exception_isr_5
	setidt $0x06, $0x08, $0x8E, $exception_isr_6
	setidt $0x07, $0x08, $0x8E, $exception_isr_7
	setidt $0x08, $0x08, $0x8E, $exception_isr_8
	setidt $0x09, $0x08, $0x8E, $exception_isr_9
	setidt $0x0A, $0x08, $0x8E, $exception_isr_10
	setidt $0x0B, $0x08, $0x8E, $exception_isr_11
	setidt $0x0C, $0x08, $0x8E, $exception_isr_12
	setidt $0x0D, $0x08, $0x8E, $exception_isr_13
	setidt $0x0E, $0x08, $0x8E, $exception_isr_14
	setidt $0x0F, $0x08, $0x8E, $exception_isr_15
	setidt $0x10, $0x08, $0x8E, $exception_isr_16
	setidt $0x11, $0x08, $0x8E, $exception_isr_17
	setidt $0x12, $0x08, $0x8E, $exception_isr_18
	setidt $0x13, $0x08, $0x8E, $exception_isr_19
	setidt $0x14, $0x08, $0x8E, $exception_isr_20
	setidt $0x15, $0x08, $0x8E, $exception_isr_21
	setidt $0x16, $0x08, $0x8E, $exception_isr_22
	setidt $0x17, $0x08, $0x8E, $exception_isr_23
	setidt $0x18, $0x08, $0x8E, $exception_isr_24
	setidt $0x19, $0x08, $0x8E, $exception_isr_25
	setidt $0x1A, $0x08, $0x8E, $exception_isr_26
	setidt $0x1B, $0x08, $0x8E, $exception_isr_27
	setidt $0x1C, $0x08, $0x8E, $exception_isr_28
	setidt $0x1D, $0x08, $0x8E, $exception_isr_29
	setidt $0x1E, $0x08, $0x8E, $exception_isr_30
	setidt $0x1F, $0x08, $0x8E, $exception_isr_31
	
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
