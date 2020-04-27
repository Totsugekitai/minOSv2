#include "kernel.h"

extern const pix_format_t black;
extern const pix_format_t white;
extern const pix_format_t red;
extern const pix_format_t green;
extern const pix_format_t blue;

extern uint64_t tick;

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

    init_kheap();

    paint_background(white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system version 2");

    puts_serial("Serial ok\r\n");

    check_all_buses();
    check_ahci();

    threads_init();

    schedule_period_init(5);

    struct thread thread0 = thread_gen(task_shikaku_aka, 0, 0);
    struct thread thread1 = thread_gen(task_shikaku_ao, 0, 0);
    struct thread thread2 = thread_gen(check_ext2, 0, 0);

    thread_stack_init(&thread0);
    thread_stack_init(&thread1);
    thread_stack_init(&thread2);

    thread_run(&thread0);
    thread_run(&thread1);
    thread_run(&thread2);

    switch_context(0, thread0.rsp);

    halt();
}

