#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define LINE_SIZE (80)

struct argv_struct {
    char argv[LINE_SIZE];
    int num;
    int len;
};

struct console {
    struct {
        uint32_t x;
        uint32_t y;
    } cursor;
    struct {
        uint16_t width;
        uint16_t height;
    } size;
};

#endif
