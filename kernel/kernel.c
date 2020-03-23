#include "kernel.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

void entry_point(bootinfo_t *binfo)
{
    init_bss();
    init_serial();
    init_graphics(binfo);
    init_gdt();
    draw_square(  0,   0, 200, 200,   red);
    draw_square(100, 100, 200, 200, green);
    draw_square(200, 200, 200, 200,  blue);
    draw_square(300, 300, 200, 200, black);
    draw_square(400, 400, 200, 200, white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system.");
    halt();
}

void kmain(void)
{
    draw_square(  0,   0, 200, 200,   red);
    draw_square(100, 100, 200, 200, green);
    draw_square(200, 200, 200, 200,  blue);
    draw_square(300, 300, 200, 200, black);
    draw_square(400, 400, 200, 200, white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system.");
    halt();
}

