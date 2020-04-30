#ifndef MM_H
#define MM_H

#include <stdint.h>

struct pgtable_info {
    int pg_num;
    uint64_t *table_start;
    int pgtable_size;
};

struct malloc_header {
    struct malloc_header *next;
    uint64_t size; // This param's unit is sizeof(struct malloc_header).
};

void init_gdt(void);
extern void load_pgtable(uint64_t *pgtable);
void init_kpaging(void);

void init_kheap(void);
void *kmalloc(int size);
void *kmalloc_alignas(int size, int align_size);
void kfree(void *ptr);
void kfree_aligned(void *ptr, int align_size);

#endif
