#ifndef X86_PCI_H
#define X86_PCI_H

#include <types.h>

u16 pci_read_config_word(u32 bus, u32 slot, u32 func, u32 offset);

void pci_test();

#endif