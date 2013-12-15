#include <types.h>

#ifdef ARCH_X86
#include <kernel/arch/x86/intr/apic.h>
#include <kernel/arch/x86/intr/idt.h>
#include <kernel/arch/x86/intr/pit.h>
#endif

#ifdef ARCH_X86_64
#include <kernel/arch/x86_64/intr/apic.h>
#include <kernel/arch/x86_64/intr/idt.h>
#endif

void interrupts_init()
{
#ifdef ARCH_X86
	idt_init();
	//enableAPIC(); // Broken
__sti;
#endif
#ifdef ARCH_X86_64
	idt_init();
__sti;
#endif
}

void set_interrupt(u32 n, void *handler)
{
#ifdef ARCH_X86
	set_idt(n, 0x08, 0x8E, handler);
#endif
#ifdef ARCH_X86_64
	set_idt(n, 0x08, 0x8E, handler);
#endif
}

void timer_init(u32 quantum)
{
#ifdef ARCH_X86
	//apic_timer_init(quantum, quantum); // Broken
	pit_init(quantum);
#endif
#ifdef ARCH_X86_64
	(void)quantum;
#endif	
}