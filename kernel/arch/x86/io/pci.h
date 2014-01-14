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

extern struct pci_device pci_devices[MAX_PCI_DEVICES]; //!< Structure defining all connected PCI devices

/**
 * Enumerate all PCI devices and functions and store them into `pci_devices`
 */
void pci_enumerate(void);

/**
 * Initializes the PCI device driver
 */
void pci_init(void);

#endif