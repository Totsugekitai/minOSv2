#ifndef PIPE_H
#define PIPE_H

typedef enum pipe_state {
                         EMPTY,
                         EXIST,
                         SENDING,
                         RECEIVING,
} pipe_state;


typedef struct pipe_struct {
    char *pipe_ptr;
    pipe_state pstate;
} pipe_struct;


#define NPIPEQUEUE (0x1000) // queue is 4KB


#endif
