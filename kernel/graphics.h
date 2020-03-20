#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "../commom/boot_types.h"

extern video_info_t vinfo_g;

typedef struct pix_format_t {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t _rsvd;
} pix_format_t;

void init_graphics(bootinfo_t *binfo);
void draw_square(uint32_t ul_x, uint32_t ul_y, uint32_t x_len, uint32_t y_len, pix_format_t color);
void printstr(uint32_t ul_x, uint32_t ul_y, pix_format_t color, pix_format_t bcolor, char *s);

#endif
