#include "kernel.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

extern uint64_t tick;
extern uint64_t __kheap_start;
extern struct malloc_header base;
extern struct malloc_header *kheap;

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

    init_kheap((struct malloc_header *)&__kheap_start);

    paint_background(white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system version 2");

    puts_serial("Serial ok\r\n");

    check_all_buses();
    check_ahci();

    threads_init();

    schedule_period_init(50);

    int tid = create_thread(init, 0, 0);
    switch_context_first(tid);

    halt();
}

void jikken(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
}

void init(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    create_thread(task_shikaku_aka, 0, 0);
    create_thread(check_ext2, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_aka, 0, 0);
    create_thread(check_ext2, 0, 0);
    create_thread(jikken, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    halt();
}
