#include <stdint.h>
#include "console.h"
#include "../util.h"
#include "../graphics.h"
#include "../device/serial.h"

static struct line_struct lines[20];
static int line_index = 0;
static struct queue_char input_buf;
static struct queue_char output_buf;
static struct char_location loc = { .col = 0, .row = 0 };


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

void print_console_input(char c)
{
    if (c == 0) {
        return;
    } else {
        printchar(loc.col * 8, loc.row * 16, white, black, c);
    }
}

void print_console_output(void)
{
    
}

static inline void readline_serial(void)
{
    key = 0;
    printstr(loc.col * 8, loc.row * 16, white, black, "> ");
    enqueue_char(&input_buf, '>');
    enqueue_char(&input_buf, ' ');
    loc.col += 2;

    char c = 0;
    while (c != 0x0d && c != 0x0a) {
        c = wait_serial_input();
        if (c == 0x0d || c == 0x0a) {
            break;
        } else {
            if (!queue_char_isfull(&input_buf)) {
                enqueue_char(&input_buf, c);
                print_console_input(c);
                loc.col++;
            } else {
                puts_serial("text buf is full\n");
                continue;
            }
        }
    }
    loc.col = 0;
    loc.row++;
}

static char *console_argv[10];
static inline void parse_line(void)
{
    int count = 0;
    while (!queue_char_isempty(&input_buf)) {
        dequeue_char(&input_buf, &lines[line_index].text[count]);
        count++;
    }
    lines[line_index].len = count;

    int j = 0;
    for (int i = 0; i < lines[line_index].len; i++) {
        if (lines[line_index].text[i] == 0x00) {
            break;
        } else if (lines[line_index].text[i] == 0x20) { // if spacw
            console_argv[j] = &lines[line_index].text[i];
            lines[line_index].text[i] = 0; // convert space to null char
            j++;
        } else {
            continue;
        }
    }
}

static inline void do_command(void)
{
    char *command = console_argv[0];

    int argc = 0;
    for (int i = 0; i < 10; i++) {
        if (console_argv[i] != 0) {
            argc++;
        }
    }

    if (!strncmp(command, "echo", 5)) {
        echo(argc, &console_argv[1]);
    } else {
        enqueue_str(&output_buf, "default command");
    }
}


