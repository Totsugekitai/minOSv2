#include <stdint.h>
#include "util.h"
#include "mm.h"

#ifndef MINOS_TEST
#include "device/serial.h"
#endif

/* Segmentation */
uint64_t *gdt = (uint64_t *)0x80;

extern void load_gdt(uint64_t *base, int limit);
extern void intersegment_jump(uint16_t cs);

#ifndef MINOS_TEST
static uint64_t make_segm_desc(uint64_t type, uint64_t dpl)
{
    // base and limit is 0
    uint64_t segm_desc = 0;
    // type is set at bit43:41
    segm_desc |= (type << 41);
    // dpl is set at bit46:45
    segm_desc |= (dpl << 45);

    // other default params
    // G,L,P,S = 1, others = 0
    segm_desc |= 0x00a0900000000000ull;

    return segm_desc;
}

#define GDT_LIMIT   24      // 8(size of gdt entry) * 3 = 24
void init_gdt(void)
{
    gdt[0] = 0x00ull;               // null descriptor
    gdt[1] = make_segm_desc(5, 0);  // KERNEL_CS
    gdt[2] = make_segm_desc(1, 0);  // KERNEL_DS
    load_gdt(gdt, GDT_LIMIT);
    intersegment_jump(1 << 3);
}
#endif

/* Paging functions */
#define ENTRY_NUM   (512)
#define TABLE_SIZE  ((8) * (ENTRY_NUM))
#define PAGE_4KB    ((uint64_t)(0x1000))
void create_kpgtable(uint64_t *start_addr)
{
    int pt_num   = 512;
    int pd_num   = 512;
    int pdpt_num = 4;   // ここの数字がGBになる
    int pml4_num = 1;
    int pgtable_size = TABLE_SIZE * (pt_num + pd_num + pdpt_num + pml4_num);
    // Initialize
    memset((void *)start_addr, 0, pgtable_size);

    // create page table
    uint64_t *pml4 = start_addr;
    uint64_t *pdpt = pml4 + (ENTRY_NUM * pml4_num);
    uint64_t *pd = pdpt + (ENTRY_NUM * pdpt_num);
    uint64_t *pt = pd + (ENTRY_NUM * pd_num);

    // format PT entries
    for (int i = 0; i < pdpt_num * pd_num * pt_num; i++) {
        pt[i] = (uint64_t)(i * PAGE_4KB) | 0x003;
    }
    // format PD entries
    for (int i = 0; i < pdpt_num * pd_num; i++) {
        pd[i] = (uint64_t)(&pt[ENTRY_NUM * i]) | 0x003;
    }
    // format PDPT entries
    for (int i = 0; i < pdpt_num; i++) {
        pdpt[i] = (uint64_t)(&pd[ENTRY_NUM * i]) | 0x003;
    }
    // format PML4 entries
    for (int i = 0; i < pml4_num; i++) {
        pml4[i] = (uint64_t)(&pdpt[ENTRY_NUM * i]) | 0x003;
    }
}

#ifndef MINOS_TEST
/* Initialization of kernel page table.
 * Kernel page size is 4KB.
*/
void init_kpaging(void)
{
    uint64_t *start_addr = (uint64_t *)0x300000;
    create_kpgtable(start_addr);
    load_pgtable(start_addr);
}
#endif

int is_aligned(void *addr, int align)
{
    uint64_t addr_num = (uint64_t)addr;
    if (addr_num % align == 0 && addr_num != 0) {
        return 1;
    } else {
        return 0;
    }
}

void *align_as(void *addr, int align)
{
    uint64_t addr_num = (uint64_t)addr;
    return (void *)(addr_num + (align - (addr_num % align)));
}

struct malloc_header base = { 0, 0 };
struct malloc_header *kheap;
struct malloc_header *freep = &base;

void init_kheap(struct malloc_header *kheap_start)
{
    kheap = kheap_start;
    // zero clear
    //memset(kheap, 0, HEAP_SIZE);

    kheap[0].next = &base;
    kheap[0].size = HEAP_SIZE / sizeof(struct malloc_header);
    base.next = kheap;
    base.size = 0;
}

void *kmalloc(int size)
{
    uint64_t nunits = ((size + sizeof(struct malloc_header) - 1) / sizeof(struct malloc_header)) + 1;

    // search free point
    struct malloc_header *p, *q = freep;
    //if ((q = freep) == 0) {
    //    freep = q = &base;
    //    base.size = 0;
    //    base.next = kheap;
    //    base.next->size = HEAP_SIZE / sizeof(struct malloc_header);
    //    base.next->next = &base;
    //}
    for (p = q->next;; q = p, p = p->next) {
        if (p->size >= nunits) { // 空きブロックが見つかった
            if (p->size == nunits) { // サイズがピッタリ
                q->next = p->next;
            } else {                 // サイズがデカい
                p->size -= nunits;
                p += p->size;
                p->size = nunits;
            }
            freep = q;
            return ((void *)(p + 1));
        }
        if (p == freep) {
            return 0;
        }
    }
    return 0;
}

struct malloc_header *get_aligned_address(struct malloc_header *start, int block_length, int align_size)
{
    struct malloc_header *i;
    for (i = start + block_length - 1; i > start; i--) {
        if ((uint64_t)i % align_size == 0) {
            return i;
        }
    }
    return 0;
}

void *kmalloc_alignas(int size, int align_size)
{
    if (align_size <= 16) {
        return kmalloc(size);
    }

    uint64_t nunits = ((size + sizeof(struct malloc_header) - 1) / sizeof(struct malloc_header)) + 1;

    // search free point
    struct malloc_header *p, *q = freep;

    struct malloc_header *aligned_addr = 0;
    for (p = q->next;; q = p, p = p->next) {
        // 空きブロック中にアライメントされたアドレスがあるかどうか
        if ((aligned_addr = get_aligned_address(p, p->size, align_size))) {
            if (aligned_addr - 1 + nunits <= p + p->size) { // 十分な大きさか
                if (aligned_addr - 1 + nunits == p + p->size) {//後ろがピッタリ
                    p->size -= nunits;
                    aligned_addr[-1].size = nunits;
                } else {                                                                      // 後ろがデカい
                    int fdiff_block = (int)(&aligned_addr[-1] - p);
                    int bdiff_block = (int)((p + p->size) - (&aligned_addr[-1] - nunits));
                    p->size = fdiff_block;
                    aligned_addr[-1].size = nunits;
                    struct malloc_header *next_p = p + fdiff_block + nunits;
                    next_p->size = bdiff_block;
                    next_p->next = p->next;
                    p->next = next_p;
                }
                freep = q;
                return (void *)aligned_addr;
            }
        }
        if (p == freep) {
            return 0;
        }
    }
    return 0;
}

void kfree(void *ptr)
{
    struct malloc_header *q; // free するブロックの直前の空きブロック
    struct malloc_header *p = (struct malloc_header *)ptr - 1; // free するブロック

    // q を探す
    for (q = freep; !(p > q && p < q->next); q = q->next) {
        if (q >= q->next && (p > q || p < q->next)) {
            break;
        }
    }

    // free するブロックの後ろが空いている場合
    if (p + p->size == q->next) {
        p->size += q->next->size;
        p->next = q->next->next;
    } else {
        p->next = q->next;
    }

    // free するブロックの前が空いている場合
    if (q + q->size == p) {
        q->size += p->size;
        q->next = p->next;
    } else {
        q->next = p;
    }

    freep = q;
}

int count_free_block(void)
{
    int count = 0;
    struct malloc_header *p = &base;
    for (;p->next != &base; p = p->next) {
        count++;
    }
    return count;
}
