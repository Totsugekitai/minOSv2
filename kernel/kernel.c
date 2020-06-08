#include "kernel.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

extern uint64_t tick;
extern uint64_t __kheap_start;

void init(int argc, char **argv);

void entry_point(bootinfo_t *binfo)
{
    tick = 0;
    init_bss();
    init_graphics(binfo);
    init_serial();
    init_gdt();
    init_kpaging();
    init_idt();
    init_pic();
    puts_serial("Serial ok\r\n");
    paint_background(white);

    init_kheap((malloc_header *)&__kheap_start);
    puts_serial("heap setting OK\n");
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system version 2");

    check_all_buses();
    check_ahci();

    threads_init();

    schedule_period_init(50);

    int tid = create_thread(init, 0, 0);
    switch_context_first(tid);

    halt();
}

void init(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    tid_t tid = fork_thread2();
    puts_serial("fork end\n");
    if (tid == -1) {
        tid = fork_thread();
        if (tid == -1) {
            console(0, 0);
        }
        puts_serial("child parent\n");
    }
    halt();
}
