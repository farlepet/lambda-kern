ENTRY (start)

SECTIONS
{
        . = 0x60000000;

        kern_start = .;
        
        .text : {
                *(.entryp)
                *(.text)
        }
        
        .rodata : {
                *(.rodata)
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
        
        intr_stack_begin = .;
        . += 0x2000;
        intr_stack_end   = .;

}