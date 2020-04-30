#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define THREAD_NUM      40   // Max threads number
#define STACK_LENGTH    0x100000

struct thread_func {
    void (*func)(int, char**);
    int argc;
    char **argv;
};

enum thread_state {
    RUNNABLE,
    WAIT,
    SLEEP,
    DEAD,
};

struct thread {
    uint64_t *stack;
    uint64_t *rsp;
    uint64_t *rip;
    struct thread_func func_info;
    enum thread_state state;
    int tid;
    int index;
};

// defined at task_asm.S
extern uint64_t *init_stack(uint64_t *stack_bottom, uint64_t *rip, struct thread *thread);
extern void switch_context(uint64_t **current_rsp, uint64_t *next_rsp);
extern void switch_context2(uint64_t **current_rsp, uint64_t *next_rsp);

int search_index_from_tid(int tid);
void threads_init(void);
void thread_stack_init(struct thread *thread);
void thread_run(struct thread *thread);
struct thread thread_gen(void (*func)(int, char**), int argc, char **argv);
int create_thread(void (*func)(int, char**), int argc, char **argv);
void schedule_period_init(uint64_t milli_sec);
void thread_scheduler(void);
void thread_scheduler2(void);
void switch_context_first(int tid);

#endif
