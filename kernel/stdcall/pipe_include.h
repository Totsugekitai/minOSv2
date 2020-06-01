#ifndef PIPE_INCLUDE_H
#define PIPE_INCLUDE_H

#include "../task.h"

int gen_pipe(thread *p);
int pread(void *buf, uint64_t buflen);
int pwrite(void *buf, uint64_t n);
int pwait(sid_t sem, pipe_state wait_state);
//int pwait_consumer(sid_t sem);
//int pwait_producer(sid_t sem);
int psignal(sid_t sem, pipe_state sig_state);
//int psignal_consumer(sid_t sem);
//int psignal_producer(sid_t sem);
int pread_await(void *buf, uint64_t buflen);
int pwrite_await(void *buf, uint64_t n);

#endif
