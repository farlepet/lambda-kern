extern pci_int_handle
global pci_interrupt

; Interrupt handler for PCI devices
pci_interrupt:
	pusha
	call pci_int_handle
	popa
	iret