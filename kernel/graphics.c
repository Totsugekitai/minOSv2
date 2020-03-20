#include <stdint.h>
#include "graphics.h"

video_info_t vinfo_g;
static int gpd = 0;

void init_graphics(bootinfo_t *binfo)
{
    vinfo_g = binfo->vinfo;

    // GPD MicroPC 用の画面補正
    if (vinfo_g.x_axis < vinfo_g.y_axis) {
        gpd = 1;
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
