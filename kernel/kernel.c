#include "kernel.h"

static const pix_format_t black = { 0x00, 0x00, 0x00, 0x00 };
static const pix_format_t white = { 0xff, 0xff, 0xff, 0x00 };
static const pix_format_t red   = { 0x00, 0x00, 0xff, 0x00 };
static const pix_format_t green = { 0x00, 0xff, 0x00, 0x00 };
static const pix_format_t blue  = { 0xff, 0x00, 0x00, 0x00 };

void entry_point(bootinfo_t *binfo)
{
    init_graphics(binfo);
    draw_square(  0,   0, 200, 200, red);
    draw_square(100, 100, 200, 200, green);
    draw_square(200, 200, 200, 200, blue);
    draw_square(300, 300, 200, 200, black);
    draw_square(400, 400, 200, 200, white);
    //uint32_t x_axis = binfo->vinfo.x_axis;
    //uint32_t y_axis = binfo->vinfo.y_axis;
    //uint32_t ppsl = binfo->vinfo.ppsl;
    //pix_format *fb = (pix_format *)binfo->vinfo.fb;

    //for (uint32_t i = 0; i < y_axis; i++) {
    //    for (uint32_t j = 0; j < x_axis; j++) {
    //        pix_format bgr = {i % 0xff, j % 0xff, (i*j) % 0xff, 0x00};
    //        fb[i * ppsl + j] = bgr;
    //    }
    //}

    while (1) {
        asm("hlt");
    }
}
