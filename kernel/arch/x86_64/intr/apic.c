#include <types.h>
#include "apic.h"

void setAPICBase(u64 apic)
{
	u32 edx = 0;
	u32 eax = (apic & 0xfffff100) | IA32_APIC_BASE_MSR_ENABLE;
	
	//#ifdef __PHYSICAL_MEMORY_EXTENSION__
	edx = (apic >> 32) & 0x0f;
	//#endif
	
	cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

u64 getAPICBase()
{
	u32 eax, edx;
	cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
	
	#ifdef __PHYSICAL_MEMORY_EXTENSION__
	return (eax & 0xfffff100) | ((edx & 0x0f) << 32);
	#else
	return (eax & 0xfffff100);
	#endif
}

void write_ioapic_register(const u64 apic_base, const u8 offset, const u32 val)
{
	/* tell IOREGSEL where we want to write to */
	*(u32*)(apic_base) = offset;
	/* write the value to IOWIN */
	*(u32*)(apic_base + 0x10) = val;
}

u32 read_ioapic_register(const u64 apic_base, const u8 offset)
{
	/* tell IOREGSEL where we want to read from */
	*(u32*)(apic_base) = offset;
	/* return the data from IOWIN */
	return *(u32*)(apic_base + 0x10);
}

void enableAPIC()
{
	/* Hardware enable the Local APIC if it wasn't enabled */
	setAPICBase(getAPICBase());
	
	/* Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts */
	write_ioapic_register(getAPICBase(), 0xF0, read_ioapic_register(getAPICBase(), 0xF0) | 0x100);
}