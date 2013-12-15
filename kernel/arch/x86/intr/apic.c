#include <dev/vga/print.h>
#include <io/ioport.h>
#include <types.h>
#include "apic.h"
#include "idt.h"

u32 apic = 0;

void setAPICBase(u32 apic)
{
	u32 edx = 0;
	u32 eax = (apic & 0xfffff100) | IA32_APIC_BASE_MSR_ENABLE;
	
	cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

u64 getAPICBase()
{
	u32 eax, edx;
	cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
	
	return (eax & 0xfffff100);
}

void write_ioapic_register(const u32 apic_base, const u8 offset, const u32 val)
{
	/* tell IOREGSEL where we want to write to */
	*(u32*)(apic_base) = offset;
	/* write the value to IOWIN */
	*(u32*)(apic_base + 0x10) = val;
}

u32 read_ioapic_register(const u32 apic_base, const u8 offset)
{
	/* tell IOREGSEL where we want to read from */
	*(u32*)(apic_base) = offset;
	/* return the data from IOWIN */
	return *(u32*)(apic_base + 0x10);
}

extern void isr_dummytmr();
extern void isr_spurious();

void apic_timer_init(u32 quantum, u32 override)
{
	u32 tmp, cpubusfreq;
	
	//set up isrs
	set_idt(32, 0x08, 0x8E, isr_dummytmr);
	set_idt(39, 0x08, 0x8E, isr_spurious);
	
	//initialize LAPIC to a well known state
	*(u32*)(apic+APIC_DFR)=0xFFFFFFFF;
	*(u32*)(apic+APIC_LDR)=((*(u32*)(apic+APIC_LDR)&0x00FFFFFF)|1);
	*(u32*)(apic+APIC_LVT_TMR)=APIC_DISABLE;
	*(u32*)(apic+APIC_LVT_PERF)=APIC_NMI;
	*(u32*)(apic+APIC_LVT_LINT0)=APIC_DISABLE;
	*(u32*)(apic+APIC_LVT_LINT1)=APIC_DISABLE;
	*(u32*)(apic+APIC_TASKPRIOR)=0;
	
	//okay, now we can enable APIC
	//global enable
	setAPICBase(apic);
	//software enable, map spurious interrupt to dummy isr
	*(u32*)(apic+APIC_SPURIOUS)=39|APIC_SW_ENABLE;
	//map APIC timer to an interrupt, and by that enable it in one-shot mode
	*(u32*)(apic+APIC_LVT_TMR)=32;
	//set up divide value to 16
	*(u32*)(apic+APIC_TMRDIV)=0x03;
	
	if(!override)
	{
		//initialize PIT Ch 2 in one-shot mode
		//waiting 1 sec could slow down boot time considerably,
		//so we'll wait 1/100 sec, and multiply the counted ticks
		outb(0x61,(inb(0x61)&0xFD)|1);
		outb(0x43,0xB2);        // Switch to Mode 0, if you get problems!!!
		//1193180/100 Hz = 11931 = 2e9bh
		outb(0x42,0x9B);	//LSB
		inb(0x60);	//short delay
		outb(0x42,0x2E);	//MSB
		
		//reset PIT one-shot counter (start counting)
		tmp=inb(0x61)&0xFE;
		outb(0x61,(u8)tmp);		//gate low
		outb(0x61,(u8)tmp|1);		//gate high
		//reset APIC timer (set counter to -1)
		*(u32*)(apic+APIC_TMRINITCNT)=0xFFFFFFFF;
		
		//now wait until PIT counter reaches zero
		while(!(inb(0x61)&0x20));
		
		//stop APIC timer
		*(u32*)(apic+APIC_LVT_TMR)=APIC_DISABLE;
		
		//now do the math...
		cpubusfreq=((0xFFFFFFFF-*(u32*)(apic+APIC_TMRCURRCNT))+1)*16*100;
		tmp=cpubusfreq/quantum/16;
	}
	else tmp = override;
	
	//sanity check, now tmp holds appropriate number of ticks, use it as APIC timer counter initializer
	*(u32*)(apic+APIC_TMRINITCNT)=(tmp<16?16:tmp);
	//finally re-enable timer in periodic mode
	*(u32*)(apic+APIC_LVT_TMR)=32|TMR_PERIODIC;
	//setting divide value register again not needed by the manuals
	//although I have found buggy hardware that required it
	*(u32*)(apic+APIC_TMRDIV)=0x03;
}
