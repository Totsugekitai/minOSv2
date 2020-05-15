#ifndef PIPE_INCLUDE_H
#define PIPE_INCLUDE_H

#include "../task.h"
int gen_pipe(thread *p);
int pread(void *buf, uint64_t n);
int pwrite(void *buf, uint64_t n);
int waitp(sid_t sem);
int signalp(sid_t sem);

#endif
