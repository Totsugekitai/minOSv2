SECTIONS
{
    . = 0x100000;
    .text   : { *(.text) }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    __bss_start = .;
    .bss    : { *(.bss) }
    __bss_end = .;
}
