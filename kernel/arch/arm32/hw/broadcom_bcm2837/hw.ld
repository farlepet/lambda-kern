ENTRY (reset)

SECTIONS
{
        . = 0x00008000;

        kern_start = .;
        
        __text_begin = .;
        .text : {
                /* @todo Use __kernel_text_begin in place of __text_begin */
                __kernel_text_begin = .;
                *(.entryp)
                *(.text.*)
                __kernel_text_end = .;
        }
        __text_end = .;
        
        __rodata_begin = .;
        .rodata : {
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
        __rodata_end = .;
        
        __data_begin = .;
        .data : {
                __kernel_data_begin = .;
                *(.data*)
                __kernel_data_end = .;

        }
        __data_end = .;
        
        __bss_begin = .;
        .bss (NOLOAD) : {
                __kernel_bss_begin = .;
                *(COMMON)
                *(.bss .bss.*)
                __kernel_bss_end = .;
        }
        __bss_end = .;
        
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