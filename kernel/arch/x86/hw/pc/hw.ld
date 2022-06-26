ENTRY (start)

SECTIONS
{
        . = 1M;

        kern_start = .;
        
        .text BLOCK(4K) : ALIGN(4K)
        {
                __kernel_text_begin = .;
                *(.mboot)
                *(.text .text.*)
                __kernel_text_end = .;
        }
        
        .rodata BLOCK(4K) : ALIGN(4K)
        {
                __kernel_rodata_begin = .;
                *(.rodata .rodata.*)

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
        
        .data BLOCK(4K) : ALIGN(4K)
        {
                __kernel_data_begin = .;
                *(.data .data.*)
                __kernel_data_end = .;
        }
        
        .bss BLOCK(4K) : ALIGN(4K)
        {
                __kernel_bss_begin = .;
                *(COMMON)
                *(.bss .bss.*)
                __kernel_bss_end = .;
        }
        
        kern_end = .;
}