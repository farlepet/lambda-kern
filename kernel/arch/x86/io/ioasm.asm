extern pci_int_handle
global pci_interrupt

pci_interrupt:
	pusha
	call pci_int_handle
	popa
	iret