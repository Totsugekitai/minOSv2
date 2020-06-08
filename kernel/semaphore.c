#include <stdint.h>
#include "task.h"
#include "semaphore.h"
#include "util.h"

#include "device/serial.h"

extern thread *threads[];

semtable_entry semtable[NSEM];

void flush_queue_sem(sid_t sem)
{
    queue_sem *que = &semtable[sem].semqueue;
    for (int i = 0; i < NSEMQUEUE; i++) {
        que->que[i] = -1;
    }
    que->head = 0;
    que->num = 0;
}

int enqueue_sem(tid_t tid, sid_t sem)
{
    queue_sem *que = &semtable[sem].semqueue;
    if (que->num < NSEMQUEUE) {
        que->que[(que->head + que->num) % NSEMQUEUE] = tid;
        que->num++;
        return 1;
    } else {
        return 0;
    }
}

int dequeue_sem(tid_t *tid, sid_t sem)
{
    queue_sem *que = &semtable[sem].semqueue;
    if (que->num > 0) {
        *tid = que->que[que->head];
        que->head = (que->head + 1) % NSEMQUEUE;
        que->num--;
        return 1;
    } else {
        return 0;
    }
}

int get_semcount(sid_t sem)
{
    if (semtable[sem].semstate == S_USED) {
        return semtable[sem].semcount;
    } else {
        return -1;
    }
}

sid_t newsem(void)
{
    for (int i = 0; i < NSEM; i++) {
        if (semtable[i].semstate == S_FREE) {
            semtable[i].semstate = S_USED;
            return i;
        }
    }
    return -1;
}

sid_t create_sem(int init_count)
{
    if (init_count < 0) {
        return -1;
    }

    sid_t sid = newsem();
    if (sid < 0) {
        return -1;
    }

    semtable[sid].semcount = init_count;
    for (int i = 0; i < NSEMQUEUE; i++) {
        semtable[sid].semqueue.que[i] = -1;
    }
    semtable[sid].semqueue.head = 0;
    semtable[sid].semqueue.num = 0;

    return sid;
}

int wait(sid_t sem)
{
    io_cli();

    STI_RET0_IF_ERR(!isbadsem(sem));
    STI_RET0_IF_ERR(semtable[sem].semstate == S_USED);

    semtable[sem].semcount--;
    if (semtable[sem].semcount < 0) {
        tid_t tid = get_cur_thread_tid();
        STI_RET0_IF_ERR(change_state(tid, WAIT));
        thread *t = get_thread_ptr(tid);
        *t->sid_ptr = sem;
        STI_RET0_IF_ERR(enqueue_sem(tid, sem));
        putsn_serial("enqueue tid: ", tid);
        thread_scheduler();
    }

    io_sti();
    return 1;
}

int signal(sid_t sem)
{
    io_cli();
    STI_RET0_IF_ERR(!isbadsem(sem));
    STI_RET0_IF_ERR(semtable[sem].semstate == S_USED);

    if (semtable[sem].semcount < 0) {
        semtable[sem].semcount++;
        tid_t tid;
        STI_RET0_IF_ERR(dequeue_sem(&tid, sem));
        putsn_serial("dequeue tid: ", tid);
        STI_RET0_IF_ERR(change_state(tid, RUNNABLE));
        thread_scheduler();
    }

    io_sti();
    return 1;
}

int delete_sem(sid_t sem)
{
    io_cli();
    STI_RET0_IF_ERR(isbadsem(sem));
    STI_RET0_IF_ERR(semtable[sem].semstate == S_FREE);

    semtable[sem].semstate = S_FREE;

    int n = semtable[sem].semqueue.num;
    for (int i = 0; i < n; i++) {
        tid_t tid;
        STI_RET0_IF_ERR(dequeue_sem(&tid, sem));
        STI_RET0_IF_ERR(change_state(tid, RUNNABLE));
    }

    io_sti();
    return 1;
}
