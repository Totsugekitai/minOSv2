#ifndef PIPE_H
#define PIPE_H

#include <stdint.h>

typedef enum pipe_state {
                         EMPTY,
                         EXIST,
                         FULL,
                         SENDING,
                         RECEIVING,
} pipe_state;


typedef struct pipe_struct {
    char *pipe_ptr;
    pipe_state pstate;
    uint64_t datalen;
} pipe_struct;


#define NPIPEQUEUE (0x1000) // queue is 4KB


#endif
