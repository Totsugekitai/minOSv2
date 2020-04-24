#include <stdint.h>
#include "console.h"
#include "../util.h"
#include "../graphics.h"

struct queue_char text_buf;
struct char_location loc = { .col = 0, .row = 0 };

extern pix_format_t black;
extern pix_format_t white;

void print_char(struct queue_char *text_buf, struct char_location *loc)
{
    printstr(loc->col * 8, loc->row * 16, white, black, &text_buf->data[text_buf->head + loc->col]);
}
