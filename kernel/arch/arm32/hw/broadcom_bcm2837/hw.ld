ENTRY (reset)

SECTIONS
{
        . = 0x80000;

        kern_start = .;
        
        .text : {
                *(.entryp)
                *(.text)
        }
        
        .rodata : {
                *(.rodata)
                
                __lambda_symbols_begin = .;
                *(.__lambda_symbols)
                __lambda_symbols_end = .;

                __lambda_strings_begin = .;
                *(.__lambda_strings)
                __lambda_strings_end = .;
        }
        
        .data : {
                *(.data)
        }
        
        .bss : {
                *(COMMON)
                *(.bss)
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