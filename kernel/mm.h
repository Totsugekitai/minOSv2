#ifndef MM_H
#define MM_H

#include <stdint.h>

struct pgtable_info {
    int pg_num;
    uint64_t *table_start;
    int pgtable_size;
};

void init_gdt(void);
extern void load_pgtable(uint64_t *pgtable);
void init_kpaging(void);

#endif
