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

    paint_background(white);
    printstr(0, 0, black, white, "minOSv2 - A minimal operating system version 2");

    puts_serial("Serial ok\r\n");

    check_all_buses();
    check_ahci();

    threads_init();

    schedule_period_init(5);

    uint64_t stack0[STACK_LENGTH];
    uint64_t stack1[STACK_LENGTH];
    struct thread thread0 = thread_gen(task_shikaku_aka, 0, 0, stack0);
    struct thread thread1 = thread_gen(task_shikaku_ao, 0, 0, stack1);

    thread_stack_init(&thread0);
    thread_stack_init(&thread1);

    thread_run(&thread0);
    thread_run(&thread1);

    switch_context(0, thread0.rsp);

    halt();
}

