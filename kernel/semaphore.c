#include <stdint.h>
#include "semaphore.h"
#include "util.h"
#include "task.h"

semtable_entry semtable[NSEMTABLE];

sid_t newsem(void)
{
    for (int i = 0; i < NSEMTABLE; i++) {
        if (semtable[i].semstate == S_FREE) {
            semtable[i].semstate = S_USED;
            return i;
        }
    }
    return -1;
}

sid_t create_sem(int init_count)
{
    if (init_count < 0) {
        return -1;
    }

    sid_t sid = newsem();
    if (sid < 0) {
        return -1;
    }

    semtable[sid].semcount = init_count;
    return sid;
}

int wait(sid_t sem)
{
    
}
