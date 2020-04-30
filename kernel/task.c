#include <stdint.h>
#include "task.h"
#include "mm.h"
#include "device/serial.h"
#include "util.h"

extern uint64_t tick;

struct thread *threads[THREAD_NUM];
int cur_thread_index = 0;
int tid_global = 0;

// for tid -> index conversion
static int tid_index_dict[THREAD_NUM] = {-1};

static struct thread empty_thread = {
                                     .stack = 0,
                                     .rsp = 0,
                                     .rip = 0,
                                     .func_info.func = 0,
                                     .func_info.argc = 0,
                                     .func_info.argv = 0,
                                     .state = DEAD,
                                     .tid = -1,
                                     .index = -1,
};

/** 周期割り込みの設定
 * timer_period: 周期割り込みの周期
 * previous_interrupt: 前回の周期割り込みの時間
*/
uint64_t timer_period = 0;
uint64_t previous_interrupt = 0;

int search_index_from_tid(int tid)
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

/** threadsの初期化処理
 * DEADのスレッドを作ってそれで埋めておく
 */
void threads_init(void)
{
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i] = &empty_thread;
    }
}

void thread_stack_init(struct thread *thread)
{
    putsp_serial("thread address: ", thread);
    thread->rsp = init_stack(thread->rsp, thread->rip, thread);
    putsn_serial("thread stack bottom: ", (uint64_t)thread->stack + STACK_LENGTH);
    putsp_serial("thread rip: ", thread->rip);
    putsp_serial("thread func address: ", thread->func_info.func);
    putsp_serial("thread rsp: ", thread->rsp);
}

/* スレッドの実行
 * threadsにスレッドを登録し、スレッドのメイン関数を実行
 * スタックに積まれた初期レジスタをpopし、
 * スタックの底に積んであるメイン関数のアドレスをもとにret命令でメイン関数に移行
 */
void thread_run(struct thread *thread)
{
    // threadsにthreadの登録をする
    int i;
    for (i = 0; i < THREAD_NUM; i++) {
        // threadsのi番目が開いていたら...
        if (threads[i]->state == DEAD) {
            threads[i] = thread;            // スレッドをi番目に入れて
            threads[i]->state = RUNNABLE;   // stateはRUNNABLE
            threads[i]->index = i;          // indexを登録
            threads[i]->tid = tid_global;
            tid_index_dict[i] = threads[i]->tid;
            tid_global++;
            putsn_serial("thread run! tid: ", threads[i]->tid);
            break;
        }
    }
}

/* スレッドの終了
 * stateをDEADにし、スケジューラに切り替える
 */
static void thread_end(int thread_index)
{
    threads[thread_index] = &empty_thread;
    tid_index_dict[thread_index] = -1;
    puts_serial("THREAD END!!\r\n");
}

static void thread_exec(struct thread *thread)
{
    //putsp_serial("thread func info into thread_exec: ", thread->func_info.func);
    thread->func_info.func(thread->func_info.argc, thread->func_info.argv);
    kfree_aligned(thread->stack, 16);
    kfree_aligned(thread, sizeof(struct thread));
    thread_end(thread->index);
    thread_scheduler();
}

/** スレッドの生成
 * thread構造体の初期化とスタックの初期化を行う
 */
struct thread thread_gen(void (*func)(int, char**), int argc, char **argv)
{
    struct thread thread;

    uint64_t *stack = kmalloc_alignas(STACK_LENGTH, 16);
    thread.stack = stack;
    thread.rsp = (uint64_t *)(thread.stack + STACK_LENGTH);
    thread.rip = (uint64_t *)thread_exec;
    thread.func_info.func = func;
    thread.func_info.argc = argc;
    thread.func_info.argv = argv;

    thread_stack_init(&thread);
    //putsn_serial("thread stack bottom: ", (uint64_t)thread.stack + STACK_LENGTH);
    //putsp_serial("thread rsp: ", thread.rsp);

    return thread;
}

void thread_gen2(struct thread *thread, void (*func)(int, char**), int argc, char **argv)
{
    uint64_t *stack = kmalloc_alignas(STACK_LENGTH, 16);
    thread->stack = stack;
    thread->rsp = (uint64_t *)(thread->stack + STACK_LENGTH);
    thread->rip = (uint64_t *)thread_exec;
    thread->func_info.func = func;
    thread->func_info.argc = argc;
    thread->func_info.argv = argv;

    thread_stack_init(thread);
}

int create_thread(void (*func)(int, char**), int argc, char **argv)
{
    io_cli();
    void *mem = kmalloc_alignas(sizeof(struct thread), sizeof(struct thread));
    struct thread *t = mem;
    thread_gen2(t, func, argc, argv);
    thread_run(t);
    io_sti();
    return t->tid;
}

void schedule_period_init(uint64_t milli_sec)
{
    timer_period = milli_sec;
}

/** This is thread scheduler
 * This function dumps current and next thread info
 * and calls switch_context
 */
void thread_scheduler(void)
{
    io_cli();
    int old_thread_index = cur_thread_index;
    // update current_index
    int i = 1;
    while (cur_thread_index == old_thread_index) {
        if (threads[(cur_thread_index + i) % THREAD_NUM]->state == RUNNABLE) {
            cur_thread_index = (cur_thread_index + i) % THREAD_NUM;
            break;
        }
        i++;
    }

    // save previous thread's rip to struct thread
    //putsn_serial("next thread index: ", current_index);
    //putsp_serial("next start rsp: ", threads[current_index]->rsp);
    //putsp_serial("next start func address: ", (uint64_t *)(threads[current_index]->func_info.func));
    //puts_serial("\r\n");
    //puts_serial("dispatch start\r\n");
    //puts_serial("\r\n");
    putsn_serial("old_thread_index: ", old_thread_index);
    putsn_serial("cur_thread_index: ", cur_thread_index);
    putsp_serial("old thread rsp: ", threads[old_thread_index]->rsp);
    putsp_serial("cur thread rsp: ", threads[cur_thread_index]->rsp);

    switch_context(&threads[old_thread_index]->rsp, threads[cur_thread_index]->rsp);
}

void thread_scheduler2(void)
{
    io_cli();
    int old_thread_index = cur_thread_index;
    // update current_index
    int i = 1;
    while (cur_thread_index == old_thread_index) {
        if (threads[(cur_thread_index + i) % THREAD_NUM]->state == RUNNABLE) {
            cur_thread_index = (cur_thread_index + i) % THREAD_NUM;
            break;
        }
        i++;
    }

    // save previous thread's rip to struct thread
    //putsn_serial("next thread index: ", current_index);
    //putsp_serial("next start rsp: ", threads[current_index]->rsp);
    //putsp_serial("next start func address: ", (uint64_t *)(threads[current_index]->func_info.func));
    //puts_serial("\r\n");
    //puts_serial("dispatch start\r\n");
    //puts_serial("\r\n");
    putsn_serial("old_thread_index: ", old_thread_index);
    putsn_serial("cur_thread_index: ", cur_thread_index);
    putsp_serial("old thread rsp: ", threads[old_thread_index]->rsp);
    putsp_serial("cur thread rsp: ", threads[cur_thread_index]->rsp);

    switch_context2(&threads[old_thread_index]->rsp, threads[cur_thread_index]->rsp);
    //__asm__ volatile("hlt");
}

void switch_context_first(int tid)
{
    int index = search_index_from_tid(tid);
    //putsp_serial("switch rsp: ", threads[index]->rsp);
    switch_context(0, threads[index]->rsp);
}
