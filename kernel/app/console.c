#include <stdint.h>
#include "console.h"
#include "command.h"
#include "../util.h"
#include "../graphics.h"
#include "../device/serial.h"
#include "../stdcall/pipe_include.h"

extern pix_format_t black;
extern pix_format_t white;

char oldkey = 0, shift = 0;
extern char key;

static char wait_serial_input(void)
{
    key = 0;
    while (key == 0) {
        __asm__ volatile("hlt");
    }
    return key;
}

static void print_char_console(char c)
{
    static console_struct cons = { .cursor.x = 0, .cursor.y = 0,
                                   .size.width = CONS_WIDTH, .size.height = CONS_HEIGHT };
    if (cons.cursor.x % CONS_WIDTH == 0 && cons.cursor.x != 0) {
        cons.cursor.x = 0;
        cons.cursor.y++;
    }
    int X = cons.cursor.x * 8;
    int Y = cons.cursor.y * 16;
    printchar(X, Y, white, black, c);
    cons.cursor.x++;
}

static void input_text_from_stdin(char *input)
{
    int i = 0;
    while (1) {
        char c = wait_serial_input();
        if (c == 0x7f) { // DEL ASCII
            i--;
            input[i] = 0;
        } else if (c == 0x0d || c == 0x0a) { // CRLF
            input[i] = c;
            return;
        } else {
            input[i] = c;
            print_char_console(c);
        }
    }
}

static void parse_input_text(char input_text[], argv_struct *argv_s)
{
    int argv_index = 0;
    for (int i = 0; input_text[i] != 0; i++) {
        if (input_text[i] == 0x0a || input_text[i] == 0x0d) {
            if (argv_s->argv[argv_index - 1] == 0) {
                continue;
            } else {
                argv_s->argv[argv_index] = 0;
                argv_index++;
                argv_s->num++;
            }
        } else {
            argv_s->argv[argv_index] = input_text[i];
            argv_index++;
        }
    }
    argv_s->len = argv_index;
}

static void do_command(int argc, char *argv[])
{
    if (strcmp(argv[0], "echo") == 0) {
        echo(argc, &argv[1]);
    } else {
        thread *t = get_thread_ptr(get_cur_thread_tid());
        pwait_producer(*t->sid_ptr);
        pwrite("No such command.", sizeof("No such command."));
        psignal_producer(*t->sid_ptr);
    }
}

static void output_text_to_stdout(char *s)
{
    for (int i = 0; s[i] != 0; i++) {
        print_char_console(s[i]);
    }
}

static void execute_command(argv_struct *argv_s, char output_text[])
{
    int argc = argv_s->num;
    int n = argc;
    int i = 0;
    char *argv[argc];
    while (n != 0) {
        if (argv_s->argv[i] != 0 && argv_s->argv[i-1] == 0) {
            argv[argc - n] = &argv_s->argv[i];
            n--;
        }
        i++;
    }

    thread *curt = get_thread_ptr(get_cur_thread_tid());
    int success = gen_pipe(curt);
    if (!success) {
        puts_serial("pipe allocate failed\n");
        halt();
    }

    puts_serial("fork console\n");
    tid_t tid = fork_thread();
    puts_serial("fork end\n");
    if (tid == -1) {
        puts_serial("child\n");
        do_command(argc-1, argv);
    } else {
        puts_serial("parent\n");
        int rest = 0;
        do {
            rest = pread_await(output_text, 100*80);
            output_text_to_stdout(output_text);
        } while (rest);
    }
}

void console(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    while (1) {
        char input_text[80] = {0};
        input_text_from_stdin(input_text);

        argv_struct argv_s = { .argv = {0}, .num = 0, .len = 0 };
        parse_input_text(input_text, &argv_s);

        char output_text[100*80] = {0};
        execute_command(&argv_s, output_text);
    }
}
