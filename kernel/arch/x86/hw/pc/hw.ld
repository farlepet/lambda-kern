ENTRY (start)

SECTIONS
{
        . = 1M;

        kern_start = .;
        
        .text BLOCK(4K) : ALIGN(4K)
        {
                *(.mboot)
                *(.text .text.*)
        }
        
        .rodata BLOCK(4K) : ALIGN(4K)
        {
                *(.rodata .rodata.*)

                __lambda_symbols_begin = ALIGN(8);
                *(.__lambda_symbols)
                __lambda_symbols_end = .;

                __lambda_strings_begin = ALIGN(8);
                *(.__lambda_strings)
                __lambda_strings_end = .;
        }
        
        .data BLOCK(4K) : ALIGN(4K)
        {
                *(.data .data.*)
        }
        
        .bss BLOCK(4K) : ALIGN(4K)
        {
                *(COMMON)
                *(.bss .bss.*)
        }
        
        kern_end = .;
}