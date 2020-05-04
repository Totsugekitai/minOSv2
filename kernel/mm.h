#ifndef MM_H
#define MM_H

#include <stdint.h>
#define HEAP_SIZE (0x400000)

struct pgtable_info {
    int pg_num;
    uint64_t *table_start;
    int pgtable_size;
};

struct malloc_header {
    struct malloc_header *next;
    uint64_t size; // This param's unit is sizeof(struct malloc_header).
};
#define BLK_SIZE (sizeof(struct malloc_header))

void init_gdt(void);
extern void load_pgtable(uint64_t *pgtable);
void init_kpaging(void);

int is_aligned(void *addr, int align);
void *alignas(void *addr, int align);
void init_kheap(struct malloc_header *kheap_start, struct malloc_header *base);
void *kmalloc(int size);
void *kmalloc_alignas(int size, int align_size);
void kfree(void *ptr);
void kfree_aligned(void *ptr, int align_size);

#endif
