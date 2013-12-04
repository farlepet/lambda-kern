#ifndef IDT_H
#define IDT_H

#define IDT_ENTRY(off, sel, attr)     \
	(((off  & 0xFFFF0000ULL) << 32) | \
	 ((attr & 0x000000FFULL) << 40) | \
	 ((sel  & 0x0000FFFFULL) << 16) | \
	 ((off  & 0x0000FFFFULL) << 0))

/*
 * IDT types:
 * 0x5 80386 32 bit Task gate
 * 0x6 80286 16-bit interrupt gate
 * 0x7 80286 16-bit trap gate
 * 0xE 80386 32-bit interrupt gate
 * 0xF 80386 32-bit trap gate
 */

#define IDT_ATTR(present, dpl, storeseg, type) \
	(((present & 0x01) << 7) | \
	((dpl      & 0x03) << 5) | \
	((storeseg & 0x01) << 4) | \
	((type     & 0x0F) << 0))


// Initialize the IDT
void idt_init();

// Set a descriptor in the IDT
void set_idt(int, int, int, void *);

#endif