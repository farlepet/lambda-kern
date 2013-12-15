%define APIC_EOI 0x0B0

extern apic
global isr_dummytmr
global isr_spurious
extern vga_print

isr_dummytmr:
	cli
	push eax
	mov eax, [apic]
	mov dword [eax+APIC_EOI], 0
	pop eax
	sti
	iret

isr_spurious:
	iret