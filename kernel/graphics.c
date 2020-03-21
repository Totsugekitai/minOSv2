#include <stdint.h>
#include "graphics.h"
#include "fonts.h"

video_info_t vinfo_g;
static int gpd = 0;

void init_graphics(bootinfo_t *binfo)
{
    vinfo_g = binfo->vinfo;

    // GPD MicroPC 用の画面補正
    if (vinfo_g.x_axis < vinfo_g.y_axis) {
        gpd = 1;
    } else {
        gpd = 0;
    }
}

static inline void draw_dot(uint32_t x, uint32_t y, pix_format_t color)
{
    pix_format_t *fb = (pix_format_t *)vinfo_g.fb;
    uint32_t x_axis = vinfo_g.x_axis;
    uint32_t y_axis = vinfo_g.y_axis;
    uint32_t ppsl = vinfo_g.ppsl;

    if (!(x < x_axis) || !(y < y_axis)) {
        return;
    }

    // GPD MicroPC 補正が入るかどうか
    uint32_t x_c = x;
    uint32_t y_c = y;
    if (gpd) {
        x_c = x_axis - y - 1;
        y_c = x;
    }
    fb[x_c + y_c * ppsl] = color;
}

void draw_square(uint32_t ul_x, uint32_t ul_y, uint32_t x_len, uint32_t y_len, pix_format_t color)
{
    uint32_t x_axis = vinfo_g.x_axis;
    uint32_t y_axis = vinfo_g.y_axis;

    if (!(ul_x + x_len < x_axis) || !(ul_y + y_len < y_axis)) {
        return;
    }

    uint32_t i, j;
    for (i = 0; i < y_len; i++) {
        for (j = 0; j < x_len; j++) {
            draw_dot(ul_x + j, ul_y + i, color);
        }
    }
}

static inline void printchar(uint32_t ul_x, uint32_t ul_y, pix_format_t color, pix_format_t bcolor, char c)
{
    char d, c_num = c - 0x20;
    int i, j;

    //io_cli();   // 割り込み禁止
    // paint background
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            draw_dot(ul_x + j, ul_y + i, bcolor);
        }
    }

    // draw character
    for (i = 0; i < 16; i++) {
        d = fonts[c_num * 16 + i];
        if ((d & 0x80) != 0) { draw_dot(ul_x + 0, ul_y + i, color); }
        if ((d & 0x40) != 0) { draw_dot(ul_x + 1, ul_y + i, color); }
        if ((d & 0x20) != 0) { draw_dot(ul_x + 2, ul_y + i, color); }
        if ((d & 0x10) != 0) { draw_dot(ul_x + 3, ul_y + i, color); }
        if ((d & 0x08) != 0) { draw_dot(ul_x + 4, ul_y + i, color); }
        if ((d & 0x04) != 0) { draw_dot(ul_x + 5, ul_y + i, color); }
        if ((d & 0x02) != 0) { draw_dot(ul_x + 6, ul_y + i, color); }
        if ((d & 0x01) != 0) { draw_dot(ul_x + 7, ul_y + i, color); }
    }
    //io_sti();   // 割り込み許可
}

void printstr(uint32_t ul_x, uint32_t ul_y, pix_format_t color, pix_format_t bcolor, char *s)
{
    int i = 0;
    while (s[i] != '\0') {
        printchar(ul_x + i * 8, ul_y, color, bcolor, s[i]);
        i++;
    }
}

