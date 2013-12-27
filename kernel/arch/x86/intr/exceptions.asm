; Contains all exception handlers
; NOTE: These are very basic handlers, most of them just hang and do nothing.
;           These should be replaced by more sophisticated handlers when the kernel boots further.

extern vga_print
extern vga_printnum
extern set_idt

%macro setidt 4
	pusha
	push %4
	push %3
	push %2
	push %1
	call set_idt
	add esp, 16
	popa
%endmacro

%macro print 1
	pusha
	push %1
	call vga_print
	add esp, 8
	popa
%endmacro

%macro printnum 1
	pusha
	print m_num_hex
	push 16
	mov eax, %1 ; Incase its a control reg
	push eax
	call vga_printnum
	add esp, 12
	print m_num_white
	popa
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



; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
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
	pop dword [errcode]
	
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
	pop dword [errcode]
	
	jmp hang

e_segnpres:
	cli
	pusha
	print m_segnpres
	popa
	pop dword [errcode]
	
	jmp hang

e_stacksegfault:
	cli
	pusha
	print m_stacksegfault
	popa
	pop dword [errcode]
	
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
	pop dword [errcode]
	
	jmp hang

extern handle_page_fault
e_pagefault:
	cli
	pusha
	print m_pagefault
	popa
	pop dword [errcode]
	pusha
	call handle_page_fault
	popa
	iret
	;jmp hang

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
	pop dword [errcode]
	
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


extern serial_write

reg_dump:
	pusha


	print m_num_newline
	print m_regdump_head
	
	print m_eax
	printnum eax
	print m_ebx
	printnum ebx
	print m_ecx
	printnum ecx
	print m_edx
	printnum edx
	
	print m_num_newline
	
	print m_cr0
	printnum cr0
	print m_cr2
	printnum cr2
	print m_cr3
	printnum cr3
	print m_cr4
	printnum cr4
	
	print m_num_newline
	
	print m_esi
	printnum esi
	print m_edi
	printnum edi
	
	print m_num_newline
	
	print m_cs
	printnum cs
	print m_ds
	printnum ds
	print m_ss
	printnum ss
	
	print m_num_newline
	
	print m_err
	printnum [errcode]
	
	print m_num_newline
	
	popa
	ret



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

errcode:   dd 0
oldcs:     dw 0
oldeip:    dd 0
oldeflags: dd 0
oldss:     dd 0
oldesp:    dd 0

m_regdump_head:  db `-------- Registers -------\n`, 0
m_eax:           db "EAX: ", 0
m_ebx:           db "EBX: ", 0
m_ecx:           db "ECX: ", 0
m_edx:           db "EDX: ", 0
m_cr0:           db "CR0: ", 0
m_cr2:           db "CR2: ", 0
m_cr3:           db "CR3: ", 0
m_cr4:           db "CR4: ", 0
m_esi:           db "ESI: ", 0
m_edi:           db "EDI: ", 0
m_cs:            db "CS: ", 0
m_ds:            db "DS: ", 0
m_ss:            db "SS: ", 0
m_err:           db "ERRORCODE: ", 0
m_num_hex:       db "0x", 0
m_num_white:     db ` \t`, 0
m_num_newline:   db `\n`, 0