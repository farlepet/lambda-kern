extern pci_int_handle
global pci_interrupt

; Interrupt handler for PCI devices
pci_interrupt:
	pusha
	call pci_int_handle
	popa
	iret


extern serial_int_handle
global serial_interrupt

; Interrupt handler for serial devices
serial_interrupt:
	pusha
	call serial_int_handle
	popa
	iret
