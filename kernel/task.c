#include <stdint.h>
#include "task.h"

extern uint64_t tick;

static struct thread *threads[THREAD_NUM];
static int current_index = 0;

static struct thread empty_thread;

/** 周期割り込みの設定
 * timer_period: 周期割り込みの周期
 * previous_interrupt: 前回の周期割り込みの時間
*/
uint64_t timer_period = 0;
uint64_t previous_interrupt = 0;
