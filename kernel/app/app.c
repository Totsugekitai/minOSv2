#include <stdint.h>
#include "../graphics.h"
#include "../util.h"
#include "../device/serial.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

void task_shikaku_aka(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    uint32_t x = 0;

    while (1) {
        printstr((x-8)%400 + 200, 80, white, white, " ");
        printstr(x%400 + 200, 80, red, red, " ");
        stihlt();
        x++;
    }
}

void task_shikaku_ao(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    uint32_t x = 0;
    while (1) {
        printstr((x-8)%400 + 200, 100, white, white, " ");
        printstr(x%400 + 200, 100, blue, blue, " ");
        stihlt();
        x++;
    }
}

