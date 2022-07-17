PHYS_ENTRYPOINT = 0x00008000;
VIRT_OFFSET     = 0xC0000000;

ENTRY(reset)

SECTIONS
{
        . = PHYS_ENTRYPOINT + VIRT_OFFSET;

        kern_start = .;

        .text : AT(ADDR(.text) - VIRT_OFFSET) {
                __kernel_text_begin = .;
                *(.entryp)
                *(.text.*)
                __kernel_text_end = .;
        }

        .rodata : AT(ADDR(.rodata) - VIRT_OFFSET) {
                __kernel_rodata_begin = .;
                *(.rodata.*)

                . = ALIGN(8);
                __lambda_symbols_begin = .;
                *(.__lambda_symbols)
                __lambda_symbols_end = .;

                . = ALIGN(8);
                __lambda_strings_begin = .;
                *(.__lambda_strings)
                __lambda_strings_end = .;

                __kernel_rodata_end = .;
        }

        .data : AT(ADDR(.data) - VIRT_OFFSET) {
                __kernel_data_begin = .;
                *(.data*)
                __kernel_data_end = .;

        }

        .bss (NOLOAD) : AT(ADDR(.bss) - VIRT_OFFSET) {
                __kernel_bss_begin = .;
                *(COMMON)
                *(.bss .bss.*)
                __kernel_bss_end = .;
        }

        kern_end = .;

        new_stack_begin = .;
        . += 0x10000;
        new_stack_end   = .;
        
        fiq_stack_begin = .;
        . += 0x2000;
        fiq_stack_end   = .;
        
        irq_stack_begin = .;
        . += 0x2000;
        irq_stack_end   = .;

}