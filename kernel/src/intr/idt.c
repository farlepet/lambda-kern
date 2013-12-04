#include <types.h>
#include <ioport.h>
#include <intr/idt.h>

extern void load_idt(u64 *, u32);
extern void dummy_int();

u64 IDT[256];

static void remap_pic(int off1, int off2)
{
	outb(0x20, 0x11);
  	outb(0xA0, 0x11);
  	outb(0x21, off1);
  	outb(0xA1, off2);
  	outb(0x21, 0x04);
  	outb(0xA1, 0x02);
  	outb(0x21, 0x01);
  	outb(0xA1, 0x01);
 	outb(0x21, 0x0);
  	outb(0xA1, 0x0);
}

void reload_idt()
{
	load_idt(IDT, sizeof(IDT));
	remap_pic(0x20, 0x28);
}

void idt_init()
{
	int i = 0;
	for(; i < 256; i++)
		IDT[i] = IDT_ENTRY((u32)&dummy_int, 0x08, 0x8E);
	
	reload_idt();
}

void set_idt(int intr, int sel, int flags, void *func)
{
	IDT[intr] = IDT_ENTRY((u32)func, sel, flags);
}