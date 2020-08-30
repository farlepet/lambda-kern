# Contains all exception handlers
# NOTE: These are very basic handlers, most of them just hang and do nothing.
#           These should be replaced by more sophisticated handlers when the kernel boots further.

.extern vga_print
.extern vga_printnum
.extern set_idt

.extern stub_error

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

.macro print str
	pusha
	pushl \str
	call vga_print
	addl $4, %esp
	popa
.endm

.macro printnum num
	pusha
	print m_num_hex
	pushl $16
	movl \num, %eax # Incase its a control reg
	pushl %eax
	call vga_printnum
	addl $12, %esp
	print m_num_white
	popa
.endm

.global exceptions_init
exceptions_init:
	setidt $0x00, $0x08, $0x8E, $e_div0
	setidt $0x01, $0x08, $0x8E, $e_debug
	setidt $0x02, $0x08, $0x8E, $e_nmi
	setidt $0x03, $0x08, $0x8E, $e_breakpoint
	setidt $0x04, $0x08, $0x8E, $e_overflow
	setidt $0x05, $0x08, $0x8E, $e_boundr
	setidt $0x06, $0x08, $0x8E, $e_invalidop
	setidt $0x07, $0x08, $0x8E, $e_devavail
	setidt $0x08, $0x08, $0x8E, $e_doublefault
	setidt $0x09, $0x08, $0x8E, $e_coprocover
	setidt $0x0A, $0x08, $0x8E, $e_invtss
	setidt $0x0B, $0x08, $0x8E, $e_segnpres
	setidt $0x0C, $0x08, $0x8E, $e_stacksegfault
	setidt $0x0D, $0x08, $0x8E, $e_gpf
	setidt $0x0E, $0x08, $0x8E, $e_pagefault

	setidt $0x10, $0x08, $0x8E, $e_fpe
	setidt $0x11, $0x08, $0x8E, $e_alignchk
	setidt $0x12, $0x08, $0x8E, $e_machchk
	setidt $0x13, $0x08, $0x8E, $e_simdfpe
	setidt $0x14, $0x08, $0x8E, $e_virtexcep

	setidt $0x1E, $0x08, $0x8E, $e_securexcep

	ret



# pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
e_div0:
	cli
	pusha
	print m_div0
	popa

	jmp hang

e_debug:
	cli
	pusha
	print m_debug
	popa

	jmp hang

e_nmi:
	cli
	pusha
	print m_nmi
	popa

	jmp hang

e_breakpoint:
	cli
	pusha
	print m_breakpoint
	popa

	iret

e_overflow:
	cli
	pusha
	print m_overflow
	popa

	jmp hang

e_boundr:
	cli
	pusha
	print m_boundr
	popa

	jmp hang

.extern handle_invalid_op
e_invalidop:
	cli
	pusha
	call handle_invalid_op
	#print m_invalidop
	popa
	sti
	iret
	#jmp hang

e_devavail:
	cli
	pusha
	print m_devavail
	popa

	jmp hang

.extern handle_double_fault
e_doublefault:
	cli
	#call stub_error

	pusha
	#print m_doublefault
	popa
	#pop dword [errcode]

	jmp hang

e_coprocover:
	cli
	pusha
	print m_cprocover
	popa

	jmp hang

e_invtss:
	cli
	pusha
	print m_invtss
	popa
	popl (errcode)

	jmp hang

e_segnpres:
	cli
	pusha
	print m_segnpres
	popa
	popl (errcode)

	jmp hang

e_stacksegfault:
	cli
	pusha
	print m_stacksegfault
	popa
	popl (errcode)

	jmp hang

.extern handle_gpf
e_gpf:
	cli
	popl (errcode)

	pusha
	pushl (errcode)
	call handle_gpf
	addl $4, %esp
	popa

	iret

.extern handle_page_fault
e_pagefault:
	cli
	popl (errcode)

	pusha
	mov %cr2, %eax
	pushl %eax
	pushl (errcode)
	call handle_page_fault
	addl $8, %esp
	popa

	iret


e_fpe:
	cli
	pusha
	print m_fpe
	popa

	jmp hang

e_alignchk:
	cli
	pusha
	print m_alignchk
	popa
	popl (errcode)

	jmp hang

e_machchk:
	cli
	pusha
	print m_machchk
	popa

	jmp hang

e_simdfpe:
	cli
	pusha
	print m_simdfpe
	popa

	jmp hang

e_virtexcep:
	cli
	pusha
	print m_virtexcep
	popa

	jmp hang

e_securexcep:
	cli
	pusha
	print m_securexcep
	popa

	jmp hang



hang:
	call reg_dump
  .halt:
	hlt
	jmp .halt


.extern serial_write

reg_dump:
	pusha


	print m_num_newline
	print m_regdump_head

	print m_eax
	printnum %eax
	print m_ebx
	printnum %ebx
	print m_ecx
	printnum %ecx
	print m_edx
	printnum %edx

	print m_num_newline

	print m_cr0
	printnum %cr0
	print m_cr2
	printnum %cr2
	print m_cr3
	printnum %cr3
	print m_cr4
	printnum %cr4

	print m_num_newline

	print m_esi
	printnum %esi
	print m_edi
	printnum %edi

	print m_num_newline

	print m_cs
	printnum %cs
	print m_ds
	printnum %ds
	print m_ss
	printnum %ss

	print m_num_newline

	print m_err
	printnum (errcode)

	print m_num_newline

	popa
	ret



.section .data
# Messages for various exceptions
m_div0:          .asciz "EXCEPTION: Divide by 0"
m_debug:         .asciz "EXCEPTION: Debug"
m_nmi:           .asciz "EXCEPTION: Non-Maskable Interrupt"
m_breakpoint:    .asciz "EXCEPTION: Breakpoint"
m_overflow:      .asciz "EXCEPTION: Overflow"
m_boundr:        .asciz "EXCEPTION: Bound Range Exceeded"
m_invalidop:     .asciz "EXCEPTION: Invalid Opcode"
m_devavail:      .asciz "EXCEPTION: Device Not Available"
m_doublefault:   .asciz "EXCEPTION: Double Fault"
m_cprocover:     .asciz "EXCEPTION: Coprocessor Segment Overrun"
m_invtss:        .asciz "EXCEPTION: Invalid TSS"
m_segnpres:      .asciz "EXCEPTION: Segment Not Present"
m_stacksegfault: .asciz "EXCEPTION: Stack Segment Fault"
m_gpf:           .asciz "EXCEPTION: General Protection Fault: 0x"
m_pagefault:     .asciz "EXCEPTION: Page Fault"
m_fpe:           .asciz "EXCEPTION: x87 Floating-Point Exception"
m_alignchk:      .asciz "EXCEPTION: Alignment Check"
m_machchk:       .asciz "EXCEPTION: Machine Check"
m_simdfpe:       .asciz "EXCEPTION: SIMD Floating-Point Exception"
m_virtexcep:     .asciz "EXCEPTION: Virtualization Exception"
m_securexcep:    .asciz "EXCEPTION: Security Exception"

errcode:   .long 0
address:   .long 0
oldcs:     .word 0
oldeip:    .long 0
oldeflags: .long 0
oldss:     .long 0
oldesp:    .long 0

m_regdump_head:  .asciz "-------- Registers -------\n"
m_eax:           .asciz "EAX: "
m_ebx:           .asciz "EBX: "
m_ecx:           .asciz "ECX: "
m_edx:           .asciz "EDX: "
m_cr0:           .asciz "CR0: "
m_cr2:           .asciz "CR2: "
m_cr3:           .asciz "CR3: "
m_cr4:           .asciz "CR4: "
m_esi:           .asciz "ESI: "
m_edi:           .asciz "EDI: "
m_cs:            .asciz "CS: "
m_ds:            .asciz "DS: "
m_ss:            .asciz "SS: "
m_err:           .asciz "ERRORCODE: "
m_num_hex:       .asciz "0x"
m_num_white:     .asciz " \t"
m_num_newline:   .asciz "\n"
