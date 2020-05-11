#ifndef PIPE_H
#define PIPE_H

#include "../semaphore.h"

#define NPIPEQUEUE (0x1000) // queue is 4KB

typedef struct pipetable_entry {
    sid_t sendsem;
    sid_t recvsem;
    char queue[NPIPEQUEUE];
} pipetable_entry;

#endif
