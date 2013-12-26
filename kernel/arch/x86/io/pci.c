#include "pci.h"
#include "ioport.h"
#include <types.h>
#include <err/error.h>

u16 pci_read_config_word(u32 bus, u32 slot, u32 func, u32 offset)
{
	u32 address;
 
	address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
 
	outl(0xCF8, address);

	return ((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

void pci_test()
{
	u16 tmp;

	int bus = 0;
	int slot = 0;

	for(; bus <= 255; bus++)
		for(slot = 0; slot <= 255; slot++)
		{
			tmp = pci_read_config_word(bus, slot, 0, 0); // Vendor ID
			if(tmp != 0xFFFF)
			{
				u16 vend = tmp;
				u16 dev = pci_read_config_word(bus, slot, 0, 1);
				u8  headt = pci_read_config_word(bus, slot, 0, 0x0E) & 0xFF;

				kerror(ERR_INFO, "  -> V: 0x%04X D: 0x%04X Ht: 0x%02X", vend, dev, headt);
			}
		}
}