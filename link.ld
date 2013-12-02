ENTRY (start)

SECTIONS
{
        . = 1M;

        kern_start = .;
        
        .text BLOCK(4K) : ALIGN(4K)
        {
                *(.mboot)
                *(.text)
        }
        
        .rodata BLOCK(4K) : ALIGN(4K)
        {
                *(.rodata)
        }
        
        .data BLOCK(4K) : ALIGN(4K)
        {
                *(.data)
        }
        
        .bss BLOCK(4K) : ALIGN(4K)
        {
                *(COMMON)
                *(.bss)
        }
        
        kern_end = .;
}