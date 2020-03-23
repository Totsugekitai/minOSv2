#include <stdint.h>
#include "graphics.h"
#include "fonts.h"
#include "util.h"

video_info_t vinfo_g;
static int gpd = 0;

const pix_format_t black = { 0x00, 0x00, 0x00, 0x00 };
const pix_format_t white = { 0xff, 0xff, 0xff, 0x00 };
const pix_format_t red   = { 0x00, 0x00, 0xff, 0x00 };
const pix_format_t green = { 0x00, 0xff, 0x00, 0x00 };
const pix_format_t blue  = { 0xff, 0x00, 0x00, 0x00 };

void init_graphics(bootinfo_t *binfo)
{
    vinfo_g = binfo->vinfo;

    // GPD MicroPC 用の画面補正
    if (vinfo_g.x_axis < vinfo_g.y_axis) {
        gpd = 1;
        uint32_t tmp = vinfo_g.x_axis;
        vinfo_g.x_axis = vinfo_g.y_axis;
        vinfo_g.y_axis = tmp;
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
        x_c = y_axis - y - 1;
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

void paint_background(pix_format_t bgcolor)
{
    for (uint64_t i = 0; i < vinfo_g.fb_size; i++) {
        ((pix_format_t *)(vinfo_g.fb))[i] = bgcolor;
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

void printnum(uint32_t ul_x, uint32_t ul_y, struct pix_format_t color,
        struct pix_format_t bcolor, uint64_t num)
{
    uint64_t i;
    uint64_t numarr[16];
    char numchararr[19];
    for (i = 0; i < 16; i++) {
        numarr[i] = (num & (mypow(2, 0 + i * 4) + mypow(2, 1 + i * 4) + mypow(2, 2 + i * 4) +
                        mypow(2, 3 + i * 4))) >> i * 4;
    }
    numchararr[0] = '0';
    numchararr[1] = 'x';
    numchararr[18] = '\0';
    for (i = 0; i < 16; i++) {
        if (numarr[i] == 0x0) {
            numchararr[17 - i] = '0';
        } else if (numarr[i] == 0x1) {
            numchararr[17 - i] = '1';
        } else if (numarr[i] == 0x2) {
            numchararr[17 - i] = '2';
        } else if (numarr[i] == 0x3) {
            numchararr[17 - i] = '3';
        } else if (numarr[i] == 0x4) {
            numchararr[17 - i] = '4';
        } else if (numarr[i] == 0x5) {
            numchararr[17 - i] = '5';
        } else if (numarr[i] == 0x6) {
            numchararr[17 - i] = '6';
        } else if (numarr[i] == 0x7) {
            numchararr[17 - i] = '7';
        } else if (numarr[i] == 0x8) {
            numchararr[17 - i] = '8';
        } else if (numarr[i] == 0x9) {
            numchararr[17 - i] = '9';
        } else if (numarr[i] == 0xa) {
            numchararr[17 - i] = 'a';
        } else if (numarr[i] == 0xb) {
            numchararr[17 - i] = 'b';
        } else if (numarr[i] == 0xc) {
            numchararr[17 - i] = 'c';
        } else if (numarr[i] == 0xd) {
            numchararr[17 - i] = 'd';
        } else if (numarr[i] == 0xe) {
            numchararr[17 - i] = 'e';
        } else if (numarr[i] == 0xf) {
            numchararr[17 - i] = 'f';
        } else {
            printstr(ul_x, ul_y, color, bcolor, "Invalid token");
        }
    }
    printstr(ul_x, ul_y, color, bcolor, numchararr);
}

void printstrnum(uint32_t ul_x, uint32_t ul_y, struct pix_format_t color, struct pix_format_t bcolor,
        char *s, uint64_t num)
{
    int len = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        len++;
    }
    printstr(ul_x, ul_y, color, bcolor, s);
    printnum(ul_x + len * 8, ul_y, color, bcolor, num);
}

