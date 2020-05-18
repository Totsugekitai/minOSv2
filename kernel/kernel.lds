SECTIONS
{
    . = 0x101000;
    .text   : { *(.text) }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    __bss_start = .;
    .bss    : { *(.bss) }
    __bss_end = .;
    . = ALIGN(16);
    __kheap_start = .;
}
