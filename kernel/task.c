#include <stdint.h>
#include "task.h"
#include "mm.h"
#include "device/serial.h"
#include "util.h"
#include "semaphore.h"
#include "stdcall/pipe.h"

extern uint64_t tick;

thread *threads[THREAD_NUM];
int g_cur_thread_index = 0;
tid_t tid_global = 0;

// for tid -> index conversion
static tid_t tid_index_dict[THREAD_NUM] = {-1};

static thread empty_thread = {
                              .stack = 0,
                              .stack_btm = 0,
                              .rsp = 0,
                              .rip = 0,
                              .func_info.func = 0,
                              .func_info.argc = 0,
                              .func_info.argv = 0,
                              .state = DEAD,
                              .tid = -1,
                              .ptid = -1,
                              .index = -1,
                              .sid_ptr = 0,
                              .pstruct = 0,
};

/** 周期割り込みの設定
 * timer_period: 周期割り込みの周期
 * previous_interrupt: 前回の周期割り込みの時間
*/
uint64_t g_timer_period = 0;
uint64_t g_previous_interrupt = 0;

tid_t get_cur_thread_tid(void)
{
    io_cli();
    tid_t tid = tid_index_dict[g_cur_thread_index];
    io_sti();
    return tid;
}

tid_t set_ctid(thread *t, tid_t ctid)
{
    io_cli();
    if (ctid < 0) {
        io_sti();
        return -1;
    }
    for (int i = 0; i < NTHREAD_CHILD; i++) {
        if (t->ctid[i] == -1) {
            t->ctid[i] = ctid;
            io_sti();
            return t->ctid[i];
        }
    }
    io_sti();
    return -1;
}

tid_t clear_ctid(thread *t, tid_t ctid)
{
    io_cli();
    if (ctid < 0) {
        io_sti();
        return -1;
    }
    for (int i = 0; i < NTHREAD_CHILD; i++) {
        if (t->ctid[i] == ctid) {
            t->ctid[i] = -1;
            io_sti();
            return ctid;
        }
    }
    io_sti();
    return -1;
}

int search_index_from_tid(tid_t tid)
{
    int ret = -1;
    for (int i = 0; i < THREAD_NUM; i++) {
        if (tid_index_dict[i] == tid) {
            ret = i;
            break;
        }
    }
    if (ret == -1) {
        puts_serial("no such tid thread!!\n");
    }
    return ret;
}

thread *get_thread_ptr(tid_t tid)
{
    int t_index = search_index_from_tid(tid);
    return threads[t_index];
}

int change_state(tid_t tid, thread_state state)
{
    thread *t = get_thread_ptr(tid);
    if (!t) {
        return 0;
    }
    t->state = state;
    return 1;
}

/** threadsの初期化処理
 * DEADのスレッドを作ってそれで埋めておく
 */
void threads_init(void)
{
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i] = &empty_thread;
    }
}

void thread_stack_init(thread *thread)
{
    putsp_serial("thread address: ", thread);
    thread->rsp = init_stack(thread->rsp - 1, thread->rip, thread);
    putsn_serial("thread stack bottom: ", (uint64_t)thread->stack_btm);
    putsp_serial("thread rip: ", thread->rip);
    putsp_serial("thread func address: ", thread->func_info.func);
    putsp_serial("thread rsp: ", thread->rsp);
    puts_serial("@thread_stack_init stack init end\n");
}

/* スレッドの実行
 * threadsにスレッドを登録し、スレッドのメイン関数を実行
 * スタックに積まれた初期レジスタをpopし、
 * スタックの底に積んであるメイン関数のアドレスをもとにret命令でメイン関数に移行
 */
void thread_run(thread *thread)
{
    // threadsにthreadの登録をする
    int i;
    for (i = 0; i < THREAD_NUM; i++) {
        // threadsのi番目が開いていたら...
        if (threads[i]->state == DEAD) {
            io_cli();
            threads[i] = thread;            // スレッドをi番目に入れて
            threads[i]->state = RUNNABLE;   // stateはRUNNABLE
            threads[i]->index = i;          // indexを登録
            threads[i]->ptid = get_cur_thread_tid();
            threads[i]->tid = tid_global;
            tid_index_dict[i] = threads[i]->tid;
            tid_global++;
            io_sti();
            putsn_serial("thread run! tid: ", threads[i]->tid);
            putsn_serial("           ptid: ", threads[i]->ptid);
            putsn_serial("          index: ", threads[i]->index);
            break;
        }
    }
}


static int thread_register(thread *thread)
{
    // threadsにthreadの登録をする
    for (int i = 0; i < THREAD_NUM; i++) {
        // threadsのi番目が開いていたら...
        if (threads[i]->state == DEAD) {
            io_cli();
            threads[i] = thread;            // スレッドをi番目に入れて
            threads[i]->state = SLEEP;
            threads[i]->index = i;          // indexを登録
            threads[i]->ptid = get_cur_thread_tid();
            threads[i]->tid = tid_global;
            tid_index_dict[i] = threads[i]->tid;
            tid_global++;
            io_sti();
            putsn_serial("register thread tid: ", threads[i]->tid);
            putsn_serial("               ptid: ", threads[i]->ptid);
            putsn_serial("              index: ", threads[i]->index);
            return i;
        }
    }
    return -1;
}


/* スレッドの終了
 * stateをDEADにし、スケジューラに切り替える
 */
static void thread_end(thread *t)
{
    threads[t->index] = &empty_thread;
    tid_t tid = t->tid;
    thread *parent_t = get_thread_ptr(t->ptid);
    clear_ctid(parent_t, tid);
    tid_index_dict[t->index] = -1;
    if (get_semcount(*t->sid_ptr) == 0) {
        delete_sem(*t->sid_ptr);
        kfree(t->pstruct->pipe_ptr);
        kfree(t->pstruct);
    }
    kfree(t->sid_ptr);
    kfree(t);

    puts_serial("THREAD END!!\r\n");
}

static void thread_exec(thread *thread)
{
    //putsp_serial("thread func info into thread_exec: ", thread->func_info.func);
    thread->func_info.func(thread->func_info.argc, thread->func_info.argv);
    kfree(thread->stack);
    thread_end(thread);
    thread_scheduler();
}

/** スレッドの生成
 * thread構造体の初期化とスタックの初期化を行う
 */
static void thread_gen_for_fork(thread *newt, thread *oldt, uint64_t *stack)
{
    newt->stack = stack;
    newt->stack_btm = (uint64_t *)((uint64_t)newt->stack + STACK_LENGTH);
    putsp_serial("newt->stack_btm: ", newt->stack_btm);
    putsp_serial("oldt->stack_btm: ", oldt->stack_btm);
    //for (uint64_t i = 0; i < (STACK_LENGTH / sizeof(uint64_t)); i++) {
    //    newt->stack[i] = oldt->stack[i];
    //}
    memcpy(newt->stack, oldt->stack, STACK_LENGTH);
    newt->rsp = (uint64_t *)((uint64_t)newt->stack_btm - ((uint64_t)oldt->stack_btm - (uint64_t)oldt->rsp));
    putsn_serial("stack difference: ", (uint64_t)oldt->stack_btm - (uint64_t)oldt->rsp);
    putsp_serial("oldt->rsp: ", oldt->rsp);
    putsp_serial("newt->rsp: ", newt->rsp);
    newt->rip = (uint64_t *)thread_exec;
    newt->func_info.func = oldt->func_info.func;
    newt->func_info.argc = oldt->func_info.argc;
    newt->func_info.argv = oldt->func_info.argv;
    newt->sid_ptr = oldt->sid_ptr;
    newt->pstruct = oldt->pstruct;
    memset(newt->ctid, -1, sizeof(tid_t) * NTHREAD_CHILD);
}

static void thread_gen2(thread *thread, void (*func)(int, char**), int argc, char **argv)
{
    putsp_serial("function: ", func);
    thread->stack = kmalloc_alignas(STACK_LENGTH, 16);
    thread->stack_btm = (uint64_t *)((uint64_t)thread->stack + STACK_LENGTH);
    thread->rsp = (uint64_t *)((uint64_t)thread->stack + STACK_LENGTH);
    thread->rip = (uint64_t *)thread_exec;
    thread->func_info.func = func;
    thread->func_info.argc = argc;
    thread->func_info.argv = argv;
    puts_serial("@thread_gen2 stack allocate\n");
    thread->sid_ptr = kmalloc(sizeof(sid_t));
    thread->pstruct = kmalloc(sizeof(pipe_struct));
    thread->pstruct->pstate = EMPTY;
    puts_serial("@thread_gen2 pipe allocate\n");
    memset(thread->ctid, -1, sizeof(tid_t) * NTHREAD_CHILD);

    putsp_serial("thread rip: ", thread->rip);
    thread_stack_init(thread);
    puts_serial("@thread_gen2 stack init\n");
}

tid_t create_thread(void (*func)(int, char**), int argc, char **argv)
{
    io_cli();
    thread *t = kmalloc_alignas(sizeof(thread), 16);
    thread_gen2(t, func, argc, argv);
    puts_serial("@create_thread thread_gen2 end\n");
    thread_run(t);
    io_sti();
    return t->tid;
}

void schedule_period_init(uint64_t milli_sec)
{
    g_timer_period = milli_sec;
}

/** This is thread scheduler
 * This function dumps current and next thread info
 * and calls switch_context
 */
void thread_scheduler(void)
{
    io_cli();
    int old_thread_index = g_cur_thread_index;
    // update current_index
    int i = 1;
    while (g_cur_thread_index == old_thread_index) {
        if (threads[(g_cur_thread_index + i) % THREAD_NUM]->state == RUNNABLE) {
            g_cur_thread_index = (g_cur_thread_index + i) % THREAD_NUM;
            break;
        }
        i++;
    }

    // save previous thread's rip to struct thread
    putsn_serial("old_thread_index: ", old_thread_index);
    putsn_serial("g_cur_thread_index: ", g_cur_thread_index);
    putsp_serial("old thread rsp: ", threads[old_thread_index]->rsp);
    putsp_serial("cur thread rsp: ", threads[g_cur_thread_index]->rsp);

    switch_context(&threads[old_thread_index]->rsp, threads[g_cur_thread_index]->rsp);
}

void switch_context_first(tid_t tid)
{
    int index = search_index_from_tid(tid);
    //putsp_serial("switch rsp: ", threads[index]->rsp);
    switch_context(0, threads[index]->rsp);
}

tid_t find_ctid(thread *t)
{
    for (int i = NTHREAD_CHILD - 1; i >= 0; i--) {
        if (t->ctid[i] != -1) {
            return t->ctid[i];
        }
    }
    return -1;
}

void put_thread_info(thread *t)
{
    puts_serial("=== put thread info ===\n");
    putsp_serial("stack:     ", t->stack);
    putsp_serial("stack_btm: ", t->stack_btm);
    putsp_serial("rsp:       ", t->rsp);
    putsp_serial("rip:       ", t->rip);
    putsp_serial("pipe_ptr:  ", t->pstruct->pipe_ptr);
    putsp_serial("sid_ptr:   ", t->sid_ptr);
    putsn_serial("tid:       ", t->tid);
    putsn_serial("ptid:      ", t->ptid);
    puts_serial("ctid:\n");
    for (int i = 0; i < NTHREAD_CHILD; i++) {
        if (t->ctid[i] == -1) {
            break;
        }
        putn_serial(t->ctid[i]);
        puts_serial("\n");
    }
    puts_serial("======================\n");
}

void do_fork(int argc, char *argv[])
{
    puts_serial("do_fork start\n");
    io_cli();
    if (argc != 1) {
        puts_serial("do_fork: invalid argument number\n");
        io_sti();
        return;
    }
    tid_t fork_tid = hexstr2hex(argv[0]);
    putsn_serial("fork_tid: ", fork_tid);

    uint64_t *newstack = kmalloc_alignas(STACK_LENGTH, 16);
    thread *new_thread = kmalloc_alignas(sizeof(thread), 16);
    thread *f_thread = get_thread_ptr(fork_tid);
    put_thread_info(f_thread);
    thread_gen_for_fork(new_thread, f_thread, newstack);
    thread_register(new_thread);
    set_ctid(f_thread, new_thread->tid);

    new_thread->state = RUNNABLE;
}

tid_t fork_thread2(void)
{
    io_cli();
    char tidstr[17] = {0};
    tid_t tmptid = get_cur_thread_tid();
    hex2hexstr(tmptid, tidstr);
    putsn_serial("tid: ", tmptid);
    puts_serial("tidstr: ");
    puts_serial(tidstr);
    puts_serial("\n");
    char *argv[1] = { tidstr };
    puts_serial("create thread start\n");
    tid_t fork_tid = create_thread(do_fork, 1, argv);
    puts_serial("create thread end\n");
    int fork_index = search_index_from_tid(fork_tid);
    int cur_index = search_index_from_tid(tmptid);
    g_cur_thread_index = fork_index;
    puts_serial("fork_thread switch context start\n");
    switch_context(&threads[cur_index]->rsp, threads[fork_index]->rsp);
    io_sti();

    tid_t tid = get_cur_thread_tid();
    thread *t = get_thread_ptr(tid);
    if (find_ctid(t) == -1) {
        puts_serial("child thread.\n");
        putsn_serial("g_cur_thread_index: ", g_cur_thread_index);
        puts_serial("this is child thread\n");
        //__asm__ volatile("cli");
        //__asm__ volatile("hlt");
        return -1;
    }
    puts_serial("parent thread.\n");
    putsn_serial("g_cur_thread_index: ", g_cur_thread_index);

    //__asm__ volatile("cli");
    //__asm__ volatile("hlt");
    tid_t ctid = find_ctid(t);
    putsn_serial("ctid: ", ctid);
    return ctid;
}

tid_t fork_thread(void)
{
    tid_t child_tid = 0;

    uint64_t *newstack = kmalloc_alignas(STACK_LENGTH, 16);
    thread *new_thread = kmalloc_alignas(sizeof(thread), 16);
    thread *cur_thread = get_thread_ptr(get_cur_thread_tid());
    put_thread_info(cur_thread);
    thread_gen_for_fork(new_thread, cur_thread, newstack);
    int t_index = thread_register(new_thread);
    set_ctid(cur_thread, new_thread->tid);

    child_tid = new_thread->tid;

    io_cli();
    new_thread->state = RUNNABLE;
    g_cur_thread_index = t_index;

    for (uint64_t i = 0; i < (STACK_LENGTH / sizeof(uint64_t)); i++) {
        new_thread->stack[i] = cur_thread->stack[i];
    }

    //puts_serial("switch_fork() enter\n");
    switch_fork(&cur_thread->rsp, new_thread->stack_btm, cur_thread->stack_btm);
    puts_serial("switch_fork() end\n");
    io_sti();

    io_cli();
    tid_t tid = get_cur_thread_tid();
    thread *t = get_thread_ptr(tid);
    if (find_ctid(t) == -1) {
        puts_serial("child thread.\n");
        putsn_serial("g_cur_thread_index: ", g_cur_thread_index);
        putsn_serial("t_index: ", t_index);
        puts_serial("this is child thread\n");
        //__asm__ volatile("cli");
        //__asm__ volatile("hlt");
        return -1;
    }
    puts_serial("parent thread.\n");
    putsn_serial("g_cur_thread_index: ", g_cur_thread_index);
    putsn_serial("t_index: ", t_index);
    putsn_serial("child_tid: ", child_tid);

    //__asm__ volatile("cli");
    //__asm__ volatile("hlt");
    tid_t ctid = find_ctid(t);
    putsn_serial("ctid: ", ctid);
    return ctid;
}

