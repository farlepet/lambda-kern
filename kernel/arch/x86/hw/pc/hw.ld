ENTRY (start)

PHYS_ENTRYPOINT = 0x00100000;
VIRT_OFFSET     = 0xC0000000;

SECTIONS
{
        . = PHYS_ENTRYPOINT;


        .boot BLOCK(4K) : ALIGN(4K) {
                __kernel_boot_begin = .;
                *(.mboot)
                *(.boot.*)
                __kernel_boot_end = .;
        }

        . += VIRT_OFFSET;

        kern_start = .;
        
        .text ALIGN(4K) : AT(ADDR(.text) - VIRT_OFFSET) {
                __kernel_text_begin = .;
                *(.text.*)
                __kernel_text_end = .;
        }
        
        .rodata ALIGN(4K) : AT(ADDR(.rodata) - VIRT_OFFSET) {
                __kernel_rodata_begin = .;
                *(.rodata.*)

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
        
        .data ALIGN(4K) : AT(ADDR(.data) - VIRT_OFFSET) {
                __kernel_data_begin = .;
                *(.data.*)
                __kernel_data_end = .;
        }
        
        .bss ALIGN(4K) : AT(ADDR(.bss) - VIRT_OFFSET) {
                __kernel_bss_begin = .;
                *(COMMON)
                *(.bss.*)
                __kernel_bss_end = .;
        }
        
        kern_end = .;
}