#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#define NSEM 100

// if semaphore table entry is used
#define S_FREE 0
#define S_USED 1

// semaphore table entry
struct semtblentry {
    int semstate;
    int semcount;
    int semqueue;
};

#define isbadsem(s) (s < 0 || s >= NSEM)

#endif
