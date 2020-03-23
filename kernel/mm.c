#include <stdint.h>
#include "util.h"

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

    // 他のデフォルト設定
    // G,L,P,S = 1、他は0
    segm_desc |= 0x00a0900000000000ull;

    return segm_desc;
}

#define GDT_LIMIT   24
void init_gdt(void)
{
    gdt[0] = 0x00ull;               // null descriptor
    gdt[1] = make_segm_desc(5, 0);  // KERNEL_CS
    gdt[2] = make_segm_desc(1, 0);  // KERNEL_DS
    load_gdt(gdt, GDT_LIMIT);
    intersegment_jump(1 << 3);
}
