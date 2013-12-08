#include <multiboot.h>
#include <mm/paging.h>
#include <mm/gdt.h>
#include <intr/idt.h>
#include <dev/vga/print.h>
#include <ioport.h>

extern void inttest(); //!< Simple function to test interrupts. \deprecated This is for testing only, it will be removed very soon.
extern void exceptions_init(); //!< Initializes basic exception handlers. Found in intr/exceptions.asm


/**
 * \brief Main kernel function.
 * Initializes all devices, and sets up environment.
 * @param mboot_ptr pointer to multiboot structure
 * @param initial_stack location of the initial stack
 */
int kmain(struct multiboot __unused *mboot_ptr, u32 __unused initial_stack)
{
#ifdef ARCH_X86
	vga_clear();
	gdt_init();
	idt_init();
	paging_init();
	set_idt(0x21, 0x08, 0x8E, &inttest);
	exceptions_init();
	__sti;
	
	vga_print("Welcome to Lambda OS!\n");
#endif // ARCH_X86

	for(;;);
}


/**
 * \brief A test function for the IDT.
 * \deprecated This is for testing only, it will be removed very soon.
 */
void __unused test_int()
{
	vga_print("I see the interrupts are working just fine!\n");
	outb(0x20, 0x20);
	inb(0x60);
}