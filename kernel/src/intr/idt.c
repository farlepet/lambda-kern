#include <types.h>
#include <intr/idt.h>

extern void load_idt(u64 *, u32);
extern void handle_int();
extern void dummy_int();

u64 IDT[256];

void idt_init()
{
	int i = 0;
	for(; i < 256; i++)
		IDT[i] = IDT_ENTRY((u32)&dummy_int, 0x08, 0);

	load_idt(IDT, sizeof(IDT));
}

void set_idt(int intr, int sel, int dpl, int type, void *func)
{
	if(func != 0) IDT[intr] = IDT_ENTRY((u32)func, sel, IDT_ATTR(1, dpl, 0, type));
	else IDT[intr] = IDT_ENTRY((u32)&dummy_int, 0x08, 0);
}