ENTRY (reset)

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
        
        fiq_stack_begin = .;
        . += 0x2000;
        fiq_stack_end   = .;
        
        irq_stack_begin = .;
        . += 0x2000;
        irq_stack_end   = .;

}