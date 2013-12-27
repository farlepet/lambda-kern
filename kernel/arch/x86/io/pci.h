#ifndef X86_PCI_H
#define X86_PCI_H

#include <types.h>

struct pci_device
{
	u8  bus;         //!< Bus location
	u8  slot;        //!< Slot on the bus
	u8  func;        //!< Function of the slot

	u8  header_type; //!< Header type
	u16 device_id;   //!< Device ID
	u16 vendor_id;   //!< Vendor ID
	u8  class;       //!< Class
	u8  subclass;    //!< Sub class
	u8  prog_if;     //!< Prog IF
	u8  revision_id; //!< Revision ID
	u8  latency_tmr; //!< Latency timer

	u8  interrupt;   //!< Interrupt line

	int vend_idx;    //!< Index in the vendor table
	int dev_idx;     //!< Index in the device table
	int class_idx;   //!< Index in the class table
} __packed;

#define MAX_PCI_DEVICES 64

struct pci_device pci_devices[MAX_PCI_DEVICES];

u16 pci_read_config_word(u32 bus, u32 slot, u32 func, u32 offset);

void pci_enumerate();

void pci_init();

#endif