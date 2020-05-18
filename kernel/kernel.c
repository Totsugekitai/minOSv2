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

sid_t s_global;

void forloop(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    for (int i = 0; i < 0x200; i++) {
        putsn_serial("forloop: ", i);
    }
    int success = signal(s_global);
    if (!success) {
        puts_serial("signal failed\n");
    }
}

void jikken(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    s_global = create_sem(0);
    if (s_global < 0) {
        puts_serial("create_sem failed\n");
        return;
    } else {
        putsn_serial("s_global: ", s_global);
    }
    int success = wait(s_global);
    if (!success) {
        puts_serial("wait failed\n");
        return;
    }

    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(check_ext2, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
    create_thread(task_shikaku_ao, 0, 0);
}

void do_child(void)
{
    puts_serial("func do_child()\n");
    thread *t = get_thread_ptr(get_cur_thread_tid());
    pwrite("deadbeeeeeeeeeeeeeeeeeeeeeeeeef", 32);
    int success = signalp(*t->sid_ptr);
    if (!success) {
        puts_serial("signal failed\n");
    } else {
        puts_serial("signal success\n");
    }
}

void do_parent(void)
{
    puts_serial("func do_parent()\n");
    thread *t = get_thread_ptr(get_cur_thread_tid());
    int success = waitp(*t->sid_ptr);
    if (!success) {
        puts_serial("wait failed\n");
    } else {
        char buf[32];
        pread(buf, 32);
        puts_serial(buf);
        puts_serial("receive OK\n");
    }
}

void init(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    thread *curt = get_thread_ptr(get_cur_thread_tid());
    int success = gen_pipe(curt);
    if (!success) {
        puts_serial("gen_pipe failed\n");
        halt();
    }
    tid_t tid = fork_thread();
    puts_serial("fork thread end\n");
    if (tid == -1) {
        do_child();
    } else {
        do_parent();
    }
    //create_thread(task_shikaku_aka, 0, 0);
    //create_thread(check_ext2, 0, 0);
    //create_thread(task_shikaku_ao, 0, 0);
    //create_thread(task_shikaku_aka, 0, 0);
    //create_thread(check_ext2, 0, 0);
    //create_thread(jikken, 0, 0);
    //create_thread(forloop, 0, 0);
    //create_thread(task_shikaku_ao, 0, 0);
    halt();
}
