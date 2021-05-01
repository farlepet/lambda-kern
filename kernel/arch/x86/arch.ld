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

                __lambda_symbols_begin = .;
                *(.__lambda_symbols)
                __lambda_symbols_end = .;

                __lambda_strings_begin = .;
                *(.__lambda_strings)
                __lambda_strings_end = .;
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