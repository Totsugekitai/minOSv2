#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define LINE_SIZE (80)
#define CONS_WIDTH LINE_SIZE
#define CONS_HEIGHT (40)

typedef struct argv_struct {
    char argv[LINE_SIZE];
    int num;
    int len;
} argv_struct;

typedef struct console_struct {
    struct {
        uint32_t x;
        uint32_t y;
    } cursor;
    struct {
        uint16_t width;
        uint16_t height;
    } size;
} console_struct;

void console(int argc, char *argv[]);

#endif
