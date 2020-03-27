#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define THREAD_NUM      4   // Max threads number
#define STACK_LENGTH    0x1000

struct thread_func {
    void (*func)(int, char**);
    int argc;
    char **argv;
};

enum thread_state {
    RUNNABLE,
    SLEEP,
    DEAD,
};

struct thread {
    uint64_t *stack;
    uint64_t *rsp;
    uint64_t *rip;
    struct thread_func func_info;
    enum thread_state state;
    int index;
};

extern uint64_t *init_stack(uint64_t *stack_bottom, uint64_t *rip, struct thread *thread);
extern void switch_context(uint64_t **current_rsp, uint64_t *next_rsp);

void threads_init(void);
void thread_stack_init(struct thread *thread);
void thread_run(struct thread *thread);
struct thread thread_gen(void (*func)(int, char**), int argc, char **argv, uint64_t *stack);
void schedule_period_init(uint64_t milli_sec);
void thread_scheduler(void);

#endif
