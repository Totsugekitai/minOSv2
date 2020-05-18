#include <stdint.h>
#include "console.h"
#include "../util.h"
#include "../graphics.h"
#include "../device/serial.h"

static int line_index = 0;
static struct queue_char input_buf;
static struct queue_char output_buf;

extern pix_format_t black;
extern pix_format_t white;

char oldkey = 0, shift = 0;
extern char key;

static char wait_serial_input(void)
{
    key = 0;
    while (!key) {
        __asm__ volatile("hlt");
    }
    return key;
}

static inline void print_char_console(char c)
{
    
}

static inline void input_text_from_stdin(char *input)
{
    char c;
    int i = 0;
    while (c != 0x0d && c != 0x0a) {
        c = wait_serial_input();
        input[i] = c;
        print_char_console(c);
        i++;
    }
}

void console(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    char input_text[80] = {0};
    input_text_from_stdin(input_text);

    struct argv_struct argv_s = { .argv = {0}, .num = 0, .len = 0 };
    parse_input_text(argv_s);

    char output_text[100][80] = {0};
    execute_command(argv_s, output_text);

    output_text_to_stdout(output_text);
}
