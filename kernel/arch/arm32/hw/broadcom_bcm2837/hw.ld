ENTRY (reset)

SECTIONS
{
        . = 0x00008000;

        kern_start = .;
        
        __text_begin = .;
        .text : {
                *(.entryp)
                *(.text)
        }
        __text_end = .;
        
        __rodata_begin = .;
        .rodata : {
                *(.rodata)
                
                __lambda_symbols_begin = .;
                *(.__lambda_symbols)
                __lambda_symbols_end = .;

                __lambda_strings_begin = .;
                *(.__lambda_strings)
                __lambda_strings_end = .;
        }
        __rodata_end = .;
        
        __data_begin = .;
        .data : {
                *(.data)
        }
        __data_end = .;
        
        __bss_begin = .;
        .bss : {
                *(COMMON)
                *(.bss)
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