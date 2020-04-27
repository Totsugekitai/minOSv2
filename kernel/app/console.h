#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define LINE_SIZE (80)

struct char_location {
    int col;
    int row;
};

struct line_struct {
    char text[LINE_SIZE];
    int len;
};

#endif
