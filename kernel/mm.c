#include <stdint.h>
#include "util.h"
#include "mm.h"
#include "device/serial.h"

/* Segmentation */
uint64_t *gdt = (uint64_t *)0x80;

extern void load_gdt(uint64_t *base, int limit);
extern void intersegment_jump(uint16_t cs);

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

/* Initialization of kernel page table.
 * Kernel page size is 4KB.
*/
void init_kpaging(void)
{
    uint64_t *start_addr = (uint64_t *)0x300000;
    create_kpgtable(start_addr);
    load_pgtable(start_addr);
}

#define BLK_SIZE (sizeof(struct malloc_header))
struct malloc_header base = { 0, 0 };
#define HEAP_SIZE (0x400000)
extern uint64_t __kheap_start;
struct malloc_header *kheap;
//struct malloc_header kheap[HEAP_SIZE]; // heap area
struct malloc_header *freep = 0;

void init_kheap(void)
{
    kheap = (struct malloc_header *)&__kheap_start;
    putsp_serial("kheap address: ", kheap);
    kheap[0].next = &base;
    kheap[0].size = HEAP_SIZE;
    base.next = &kheap[0];
    base.size = 0;
}

void *kmalloc(int size)
{
    uint64_t nunits = ((size + sizeof(struct malloc_header) - 1) / sizeof(struct malloc_header)) + 1;

    // search free point
    struct malloc_header *p, *q;
    if ((q = freep) == 0) {
        freep = q = &base;
        base.size = 0;
        base.next = kheap;
        base.next->size = HEAP_SIZE / sizeof(struct malloc_header);
        base.next->next = &base;
    }
    for (p = q->next;; q = p, p = p->next) {
        if (p->size >= nunits) {
            if (p->size == nunits) {
                q->next = p->next;
            } else {
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

void *kmalloc_alignas(int size, int align_size)
{
    void *tmp = kmalloc(size + align_size);
    if (is_aligned(tmp, align_size)) {
        return tmp;
    } else {
        void *true_ptr = align(tmp, align_size);
        uint64_t diff = (uint64_t)true_ptr - (uint64_t)tmp;
        putsn_serial("malloc align diff: ", diff);
        ((uint64_t *)true_ptr)[-1] = diff;
        return true_ptr;
    }
}

void kfree(void *ptr)
{
    struct malloc_header *q;
    struct malloc_header *p = (struct malloc_header *)ptr - 1;
    for (q = freep; !(p > q && p < q->next); q = q->next) {
        if (q >= q->next && (p > q || p < q->next)) {
            break;
        }
    }

    if (p + p->size == q->next) {
        p->size += q->next->size;
        p->next = q->next->next;
    } else {
        p->next = q->next;
    }

    if (q + q->size == p) {
        q->size += p->size;
        q->next = p->next;
    } else {
        q->next = p;
    }

    freep = q;
}

void kfree_aligned(void *ptr, int align_size)
{
    if ((unsigned long)align_size <= sizeof(struct malloc_header)) {
        kfree(ptr);
    } else {
        uint64_t diff = ((uint64_t *)ptr)[-1];
        void *true_ptr = (void *)((char *)ptr - diff);
        putsn_serial("kfree addr diff: ", diff);
        putsp_serial("kfree true free address point: ", true_ptr);
        kfree(true_ptr);
    }
}
