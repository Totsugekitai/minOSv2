#include <stdint.h>
#include "../mm.h"
#include "../semaphore.h"
#include "../task.h"
#include "../util.h"

int gen_pipe(thread *t)
{
    t->pstruct->pipe_ptr = kmalloc(NPIPEQUEUE);
    if (t->pstruct->pipe_ptr) {
        t->pstruct->pstate = EMPTY;
        *t->sid_ptr = create_sem(0);
        return 1;
    } else {
        return 0;
    }
}

int pread(void *buf, uint64_t n)
{
    thread *t = get_thread_ptr(get_cur_thread_tid());
    if (t->pstruct->pstate == EXIST) {
        t->pstruct->pstate = RECEIVING;
        memcpy(buf, t->pstruct->pipe_ptr, n);
        t->pstruct->pstate = EMPTY;
        return 1;
    }
    return 0;
}

int pwrite(void *buf, uint64_t n)
{
    thread *t = get_thread_ptr(get_cur_thread_tid());
    if (t->pstruct->pstate == EMPTY) {
        t->pstruct->pstate = SENDING;
        memcpy(t->pstruct->pipe_ptr, buf, n);
        t->pstruct->pstate = EXIST;
        return 1;
    }
    return 0;
}

int waitp(sid_t sem)
{
    thread *t = get_thread_ptr(get_cur_thread_tid());
    if (t->pstruct->pstate == EMPTY || t->pstruct->pstate == SENDING) {
        return wait(sem);
    } else {
        return 1;
    }
}

int signalp(sid_t sem)
{
    thread *t = get_thread_ptr(get_cur_thread_tid());
    if (t->pstruct->pstate == EXIST) {
        return signal(sem);
    } else {
        return 0;
    }
}
