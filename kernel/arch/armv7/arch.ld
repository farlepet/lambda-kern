ENTRY (start)

SECTIONS
{
        . = 0x34000000;

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
}