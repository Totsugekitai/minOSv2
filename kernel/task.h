#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define THREAD_NUM  4   // Max threads number
#define STACK_LENGTH    0x1000

struct thread_func {
    void (*func)(int, char**);
    int argc;
    char **argv;
};

enum thread_state {
    RUNNABLE,
    SLEEP,
    DEAD
};

struct thread {
    uint64_t *stack;
    uint64_t rsp;
    struct thread_func func;
    enum thread_state state;
    int index;
};

#endif
