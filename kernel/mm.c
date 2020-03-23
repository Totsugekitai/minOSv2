#include <stdint.h>
#include "util.h"
#include "mm.h"
#include "graphics.h"

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
int t_count(int n, int d)
{
    if (n % d != 0) {
        return n / d + 1;
    } else {
        return n / d;
    }
}

#define ENTRY_NUM   (512)
#define TABLE_SIZE  ((8) * (ENTRY_NUM))
#define PAGE_4KB    ((uint64_t)(0x1000))
extern const pix_format_t black;
extern const pix_format_t white;
void create_kpgtable(struct pgtable_info *table)
{
    int pg_num   = 0x200000;
    int pt_num   = 512;
    int pd_num   = 512;
    int pdpt_num = 8;
    int pml4_num = 1;
    table->pgtable_size = TABLE_SIZE * (pt_num + pd_num + pdpt_num + pml4_num);
    // Initialize
    mymemset((void *)table->table_start, 0, table->pgtable_size);

    // create page table
    uint64_t *pml4 = table->table_start;
    uint64_t *pdpt = pml4 + (ENTRY_NUM * pml4_num);
    uint64_t *pd = pdpt + (ENTRY_NUM * pdpt_num);
    uint64_t *pt = pd + (ENTRY_NUM * pd_num);
    //printstrnum(200,  0, black, white, "PML4:", (uint64_t)pml4);
    //printstrnum(200, 16, black, white, "PDPT:", (uint64_t)pdpt);
    //printstrnum(200, 32, black, white, "PD  :", (uint64_t)pd);
    //printstrnum(200, 48, black, white, "PT  :", (uint64_t)pt);
    //printstrnum(200,  64, black, white, "pg_num  :", (uint64_t)pg_num);
    //printstrnum(200,  80, black, white, "pt_num  :", (uint64_t)pt_num);
    //printstrnum(200,  96, black, white, "pd_num  :", (uint64_t)pd_num);
    //printstrnum(200, 112, black, white, "pdpt_num:", (uint64_t)pdpt_num);
    //printstrnum(200, 128, black, white, "pml4_num:", (uint64_t)pml4_num);

    // format PT entries
    for (int i = 0; i < 0x200000; i++) {
        pt[i] = (uint64_t)(i * PAGE_4KB) | 0x003;
    }
    // format PD entries
    for (int i = 0; i < 0x1000; i++) {
        pd[i] = (uint64_t)(&pt[ENTRY_NUM * i]) | 0x003;
    }
    // format PDPT entries
    for (int i = 0; i < 0x8; i++) {
        pdpt[i] = (uint64_t)(&pd[ENTRY_NUM * i]) | 0x003;
    }
    // format PML4 entries
    for (int i = 0; i < 1; i++) {
        pml4[i] = (uint64_t)(&pdpt[ENTRY_NUM * i]) | 0x003;
    }
}

/* Initialization of kernel page table.
 * Kernel page size is 4KB.
 * alloc page table covering 8GB amount of memory.
*/
uint64_t *k_pml4 = (uint64_t *)0x8000000;       // start address is 0x1000000
uint64_t *k_pdpt = (uint64_t *)0x8001000;       // 0x8001000 = 0x8000000 + 8 * 512
uint64_t *k_pd   = (uint64_t *)0x8009000;       // 0x8009000 = 0x8001000 + 8 * 512 * 8
uint64_t *k_pt   = (uint64_t *)0x8209000;       // 0x8209000 = 0x8009000 + 8 * 512 * 512
                                 // end address is 0x8409000 = 0x8209000 + 8 * 512 * 512

void init_kpaging(void)
{
    struct pgtable_info pgtable;
    pgtable.pg_num = 0x200000;
    pgtable.table_start = (uint64_t *)0x8000000;
    create_kpgtable(&pgtable);
    load_pgtable(pgtable.table_start);
}

