#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef int sid_t;

#include "task.h"

#define NSEM 100
#define NSEMQUEUE 30

// if semaphore table entry is used
#define S_FREE 0
#define S_USED 1

typedef struct queue_sem {
    int head;
    int num;
    tid_t que[NSEMQUEUE];
} queue_sem;

// semaphore table entry
typedef struct semtable_entry {
    int semstate;
    int semcount;
    queue_sem semqueue;
} semtable_entry;

#define isbadsem(s) ((s) < 0 || (s) >= NSEM)

sid_t create_sem(int init_count);
int wait(sid_t sem);
int signal(sid_t sem);
int delete_sem(sid_t sem);

#endif
