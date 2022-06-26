ENTRY (reset)

SECTIONS
{
        . = 0x60000000;

        kern_start = .;
        
        .text : {
                __kernel_text_begin = .;
                *(.entryp)
                *(.text)
                __kernel_text_end = .;
        }
        
        .rodata : {
                __kernel_rodata_begin = .;
                *(.rodata)

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
        
        .data : {
                __kernel_data_begin = .;
                *(.data)
                __kernel_data_end = .;
        }
        
        .bss : {
                __kernel_bss_begin = .;
                *(COMMON)
                *(.bss)
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