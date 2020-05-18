#ifndef TASK_H
#define TASK_H

typedef int tid_t;

#include <stdint.h>
#include "semaphore.h"
#include "stdcall/pipe.h"

#define THREAD_NUM      (40)   // Max threads number
#define NTHREAD_CHILD   (10)
#define STACK_LENGTH    (0x1000)

typedef struct thread_func {
    void (*func)(int, char**);
    int argc;
    char **argv;
} thread_func;

typedef enum thread_state {
    RUNNABLE,
    WAIT,
    SLEEP,
    DEAD,
} thread_state;

typedef struct thread {
    uint64_t *stack;
    uint64_t *stack_btm;
    uint64_t *rsp;
    uint64_t *rip;
    thread_func func_info;
    thread_state state;
    tid_t tid;
    tid_t ptid;
    tid_t ctid[NTHREAD_CHILD];
    int index;
    sid_t *sid_ptr;
    pipe_struct *pstruct;
} thread;

// defined at task_asm.S
extern uint64_t *init_stack(uint64_t *stack_bottom, uint64_t *rip, thread *thread);
extern void switch_context(uint64_t **current_rsp, uint64_t *next_rsp);
extern void switch_context2(uint64_t **current_rsp, uint64_t *next_rsp);
extern void switch_fork(uint64_t **cur_rsp, uint64_t *newstack, uint64_t *cur_stack_btm);

tid_t get_cur_thread_tid(void);
int search_index_from_tid(tid_t tid);
thread *get_thread_ptr(tid_t tid);
int change_state(tid_t tid, thread_state state);
void threads_init(void);
void thread_stack_init(thread *thread);
void thread_run(thread *thread);
thread thread_gen(void (*func)(int, char**), int argc, char **argv);
int create_thread(void (*func)(int, char**), int argc, char **argv);
void schedule_period_init(uint64_t milli_sec);
void thread_scheduler(void);
void switch_context_first(tid_t tid);
tid_t fork_thread(void);

#endif
