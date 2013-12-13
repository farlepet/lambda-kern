#include <types.h>

#ifdef ARCH_X86
#include <kernel/arch/x86/intr/idt.h>
#endif

#ifdef ARCH_X86_64
#include <kernel/arch/x86_64/intr/apic.h>
#include <kernel/arch/x86_64/intr/idt.h>
#endif

void interrupts_init()
{
#if     defined(ARCH_X86)
	idt_init();
#elseif defined(ARCH_X86_64)
	idt_init();
	//enableAPIC();
#endif
	idt_init();
}

void set_interrupt(u32 n, void *handler)
{
#if      defined(ARCH_X86)
	set_idt(n, 0x08, 0x8E, handler);
#elseif  defined(ARCH_X86_64)
	set_idt(n, 0x08, 0x8E, handler);
#endif
	set_idt(n, 0x08, 0x8E, handler);
}