#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "task.h"

typedef int sid_t;

#define NSEM 100
#define NSEMTABLE 100

// if semaphore table entry is used
#define S_FREE 0
#define S_USED 1

// semaphore table entry
typedef struct semtable_entry {
    int semstate;
    int semcount;
    tid_t semqueue;
} semtable_entry;

#define isbadsem(s) (s < 0 || s >= NSEM)

#endif
