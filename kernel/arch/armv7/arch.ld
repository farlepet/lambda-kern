ENTRY (start)

SECTIONS
{
        . = 1M;

        kern_start = .;
        
        .entryp : {
                *(.entryp)
        }

        .text : {
                *(.mboot)
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