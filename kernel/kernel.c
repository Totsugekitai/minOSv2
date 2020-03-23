#include "kernel.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

void entry_point(bootinfo_t *binfo)
{
    init_bss();
    init_graphics(binfo);
    init_serial();
    init_gdt();
    init_kpaging();
    paint_background(white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system version 2");
    printstr(200, 200, black, white, "minOSv2 - A minimal operating system version 2");
    printstr(400, 400, black, white, "minOSv2 - A minimal operating system version 2");
    printstr(600, 600, black, white, "minOSv2 - A minimal operating system version 2");
    halt();
}

void kmain(void)
{
    halt();
}

