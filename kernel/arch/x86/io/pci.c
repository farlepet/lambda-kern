#include "pci.h"
#include "pcihdr.h"
#include "ioport.h"
#include <types.h>
#include <err/error.h>
#include <video.h>
#include <intr/intr.h>


struct pci_device pci_devices[MAX_PCI_DEVICES];

int n_pci_devices;

u16 pci_read_config_word(u32 bus, u32 slot, u32 func, u32 offset)
{
	u32 address;
 
	address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
 
	outl(0xCF8, address);

	return ((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

void pci_write_config_word(u32 bus, u32 slot, u32 func, u32 offset, u16 data)
{
	u32 address;
 
	address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
 
	if(address & 2)
	{
		u16 other_side = pci_read_config_word(bus, slot, func, offset - 2);

		outl(0xCF8, address);

		outl(0xCFC, other_side | (data << 16));
	}
	else
	{
		u16 other_side = pci_read_config_word(bus, slot, func, offset + 2);

		outl(0xCF8, address);

		outl(0xCFC, data | (other_side << 16));
	}
}



void pci_enumerate()
{
	kerror(ERR_BOOTINFO, "Initializing PCI devices");
	u16 tmp;

	int bus = 0;
	int slot = 0;

	int devn = 0;

	for(; bus <= 255; bus++)
		for(slot = 0; slot <= 255; slot++)
		{
			tmp = pci_read_config_word(bus, slot, 0, 0x00); // Vendor ID
			if(tmp != 0xFFFF)
			{
				if(devn >= MAX_PCI_DEVICES)
				{
					kerror(ERR_SMERR, "Maximum PCI entries reached");
					break;
				}
				pci_devices[devn].bus  = bus;
				pci_devices[devn].slot = slot;

				u16 vend   = tmp;
				u16 dev    = pci_read_config_word(bus, slot, 0, 0x01);
				u8  headt  = pci_read_config_word(bus, slot, 0, 0x0E) & 0xFF;
				u8  class  = pci_read_config_word(bus, slot, 0, 0x0A) >> 8;
				u8  sclass = pci_read_config_word(bus, slot, 0, 0x0A) & 0xFF;
				u8  progif = pci_read_config_word(bus, slot, 0, 0x08) >> 8;
				u8  revid  = pci_read_config_word(bus, slot, 0, 0x08) & 0xFF;
				u8  lattmr = pci_read_config_word(bus, slot, 0, 0x0C) >> 8;
				u8  intln  = pci_read_config_word(bus, slot, 0, 0x3C) & 0xFF;


				pci_devices[devn].header_type = headt;
				pci_devices[devn].device_id   = dev;
				pci_devices[devn].vendor_id   = vend;
				pci_devices[devn].class       = class;
				pci_devices[devn].subclass    = sclass;
				pci_devices[devn].prog_if     = progif;
				pci_devices[devn].revision_id = revid;
				pci_devices[devn].latency_tmr = lattmr;
				pci_devices[devn].interrupt   = intln;

				devn++;

				kerror(ERR_DETAIL, "  %02X:%02X -> V: %04X D: %04X Ht: %02X C: %02X SC: %02X IF: %02X INT: %02X", bus, slot, vend, dev, headt, class, sclass, progif, intln);

				if(headt & 0x80)
				{
					int func = 1;
					for(; func < 8; func++)
					{
						tmp = pci_read_config_word(bus, slot, func, 0); // Vendor ID
						if(tmp != 0xFFFF)
						{
							u16 vend   = tmp;
							u16 dev    = pci_read_config_word(bus, slot, func, 0x01);
							u8  headt  = pci_read_config_word(bus, slot, func, 0x0E) & 0xFF;
							u8  class  = pci_read_config_word(bus, slot, func, 0x0A) >> 8;
							u8  sclass = pci_read_config_word(bus, slot, func, 0x0A) & 0xFF;
							u8  progif = pci_read_config_word(bus, slot, func, 0x08) >> 8;
							u8  revid  = pci_read_config_word(bus, slot, func, 0x08) & 0xFF;
							u8  lattmr = pci_read_config_word(bus, slot, func, 0x0C) >> 8;
							u8  intln  = pci_read_config_word(bus, slot, func, 0x3C) & 0xFF;

							pci_devices[devn].header_type = headt;
							pci_devices[devn].device_id   = dev;
							pci_devices[devn].vendor_id   = vend;
							pci_devices[devn].class       = class;
							pci_devices[devn].subclass    = sclass;
							pci_devices[devn].prog_if     = progif;
							pci_devices[devn].revision_id = revid;
							pci_devices[devn].latency_tmr = lattmr;
							pci_devices[devn].interrupt   = intln;

							devn++;

							kerror(ERR_DETAIL, "    -> F: %d V: %04X D: %04X Ht: %02X C: %02X SC: %02X IF: %02X INT: %02X", func, vend, dev, headt, class, sclass, progif, intln);
						}
					}
				}
			}
		}
	n_pci_devices = devn;
}

int bist_test(u8 bus, u8 slot, u8 func)
{
	u16 infoE = pci_read_config_word(bus, slot, func, 0x0E);

	if(!(infoE & 0x8000)) return -1; // Not BIST compatible

	pci_write_config_word(bus, slot, func, 0x0E, infoE | 0x4000);

	while(pci_read_config_word(bus, slot, func, 0x0E) & 0x4000);

	return (pci_read_config_word(bus, slot, func, 0x0E) >> 8) & 0x0F;
}

extern void pci_interrupt();

void pci_init()
{
	int i = 0;
	for(; i < n_pci_devices; i++)
	{
		u8 bus  = pci_devices[i].bus;
		u8 slot = pci_devices[i].slot;
		u8 fn   = pci_devices[i].func;
		
		int bist = bist_test(bus, slot, fn);
		if((bist != 0) && (bist != -1))
			kerror(ERR_SMERR, "BIST for %02X:%02X:%02X returned %01X", bus, slot, fn, bist);

		u16 vend = pci_devices[i].vendor_id;
		u32 n = 0;
		pci_devices[i].vend_idx = -1;
		for(; n < PCI_VENTABLE_LEN; n++)
			if(PciVenTable[n].VenId == vend)
			{
				pci_devices[i].vend_idx = n;
				break;
			}
		
		u16 dev = pci_devices[i].device_id;
		pci_devices[i].dev_idx = -1;
		for(n = 0; n < PCI_DEVTABLE_LEN; n++)
			if((PciDevTable[n].VenId == vend) && (PciDevTable[n].DevId == dev))
			{
				pci_devices[i].dev_idx = n;
				break;
			}

		u8 class   = pci_devices[i].class;
		u8 sclass  = pci_devices[i].subclass;
		u8 prog_if = pci_devices[i].prog_if;
		pci_devices[i].class_idx = -1;
		for(n = 0; n < PCI_CLASSCODETABLE_LEN; n++)
			if((PciClassCodeTable[n].BaseClass == class) && (PciClassCodeTable[n].SubClass == sclass) && (PciClassCodeTable[n].ProgIf == prog_if))
			{
				pci_devices[i].class_idx = n;
				break;
			}

		if(pci_devices[i].interrupt)
		{
			u16 val = pci_read_config_word(bus, slot, fn, 0x04);
			val &= ~(0x400); // Clear the interrupt disable flag
			pci_write_config_word(bus, slot, fn, 0x04, val);
			set_interrupt(pci_devices[i].interrupt, &pci_interrupt);
		}
	}
}



void pci_int_handle()
{
	register int i = 0;
	for(; i < n_pci_devices; i++)
	{
		if(pci_devices[i].interrupt)
		{
			u16 status = pci_read_config_word(pci_devices[i].bus, pci_devices[i].slot, pci_devices[i].func, 0x06);
			if(status & 0x08) // Check interrupt flag
			{
				// Do something
				return;
			}
		}
	}
}