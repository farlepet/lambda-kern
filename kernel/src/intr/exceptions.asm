; Contains all exception handlers
; NOTE: These are very basic handlers, most of them just hang and do nothing.
;           These should be replaced by more sophisticated handlers when the kernel boots further.

extern vga_print
extern vga_printnum
extern set_idt

%macro setidt 4
	push %4
	push %3
	push %2
	push %1
	call set_idt
	add esp, 16
%endmacro

%macro print 1
	push %1
	call vga_print
	add esp, 4
%endmacro

global exceptions_init
exceptions_init:
	setidt 0x00, 0x08, 0x8E, e_div0
	setidt 0x01, 0x08, 0x8E, e_debug
	setidt 0x02, 0x08, 0x8E, e_nmi
	setidt 0x03, 0x08, 0x8E, e_breakpoint
	setidt 0x04, 0x08, 0x8E, e_overflow
	setidt 0x05, 0x08, 0x8E, e_boundr
	setidt 0x06, 0x08, 0x8E, e_invalidop
	setidt 0x07, 0x08, 0x8E, e_devavail
	setidt 0x08, 0x08, 0x8E, e_doublefault
	setidt 0x09, 0x08, 0x8E, e_coprocover
	setidt 0x0A, 0x08, 0x8E, e_invtss
	setidt 0x0B, 0x08, 0x8E, e_segnpres
	setidt 0x0C, 0x08, 0x8E, e_stacksegfault
	setidt 0x0D, 0x08, 0x8E, e_gpf
	setidt 0x0E, 0x08, 0x8E, e_pagefault
	
	setidt 0x10, 0x08, 0x8E, e_fpe
	setidt 0x11, 0x08, 0x8E, e_alignchk
	setidt 0x12, 0x08, 0x8E, e_machchk
	setidt 0x13, 0x08, 0x8E, e_simdfpe
	setidt 0x14, 0x08, 0x8E, e_virtexcep
	
	setidt 0x1E, 0x08, 0x8E, e_securexcep

	ret



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

e_invalidop:
	cli
	pusha
	print m_invalidop
	popa
	jmp hang

e_devavail:
	cli
	pusha
	print m_devavail
	popa
	jmp hang

e_doublefault:
	cli
	pusha
	print m_doublefault
	popa
	add esp, 4 ; Remove error code from stack
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
	add esp, 4 ; Remove error code from stack
	jmp hang

e_segnpres:
	cli
	pusha
	print m_segnpres
	popa
	add esp, 4 ; Remove error code from stack
	jmp hang

e_stacksegfault:
	cli
	pusha
	print m_stacksegfault
	popa
	add esp, 4 ; Remove error code from stack
	jmp hang

e_gpf:
	cli
	push eax
	mov eax, [esp + 8]
	mov dword [errcode], eax
	pop eax
	
	pusha
	print m_gpf
	push 16
	push dword [errcode]
	call vga_printnum
	add esp, 8
	popa
	add esp, 4 ; Remove error code from stack
	jmp hang

e_pagefault:
	cli
	pusha
	print m_pagefault
	popa
	add esp, 4 ; Remove error code from stack
	jmp hang

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
	add esp, 4 ; Remove error code from stack
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
	hlt
	jmp hang



section .data
; Messages for various exceptions
m_div0:          db "EXCEPTION: Divide by 0", 0
m_debug:         db "EXCEPTION: Debug", 0
m_nmi:           db "EXCEPTION: Non-Maskable Interrupt", 0
m_breakpoint:    db "EXCEPTION: Breakpoint", 0
m_overflow:      db "EXCEPTION: Overflow", 0
m_boundr:        db "EXCEPTION: Bound Range Exceeded", 0
m_invalidop:     db "EXCEPTION: Invalid Opcode", 0
m_devavail:      db "EXCEPTION: Device Not Available", 0
m_doublefault:   db "EXCEPTION: Double Fault", 0
m_cprocover:     db "EXCEPTION: Coprocessor Segment Overrun", 0
m_invtss:        db "EXCEPTION: Invalid TSS", 0
m_segnpres:      db "EXCEPTION: Segment Not Present", 0
m_stacksegfault: db "EXCEPTION: Stack Segment Fault", 0
m_gpf:           db "EXCEPTION: General Protection Fault: 0x", 0
m_pagefault:     db "EXCEPTION: Page Fault", 0
m_fpe:           db "EXCEPTION: x87 Floating-Point Exception", 0
m_alignchk:      db "EXCEPTION: Alignment Check", 0
m_machchk:       db "EXCEPTION: Machine Check", 0
m_simdfpe:       db "EXCEPTION: SIMD Floating-Point Exception", 0
m_virtexcep:     db "EXCEPTION: Virtualization Exception", 0
m_securexcep:    db "EXCEPTION: Security Exception", 0

errcode: dd 0 ; Storage for errorcode if required