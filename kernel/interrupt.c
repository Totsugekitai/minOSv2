#include <stdint.h>
#include "int_handler.h"

struct gate_desc {
    uint32_t low;
    uint32_t mid;
    uint64_t high;
};

struct gate_desc *const idt = (struct gate_desc *)0x10000;
//struct gate_desc *const idt = (struct gate_desc *)0x13000;

struct gate_desc create_gate_desc(uint64_t handler)
{
    struct gate_desc gdesc;
    gdesc.high = (handler & 0xffffffff00000000ull) >> 32;
    gdesc.mid = (handler & 0xffff0000) | 0x8e00;
    gdesc.low = 0x80000 | (handler & 0xffff);

    return gdesc;
}

void init_idt(void)
{
    // Initialize
    struct gate_desc zero = { 0, 0, 0 };
    for (int i = 0; i < 256; i++) {
        idt[i] = zero;
    }

    // register exception handlers
    idt[0] = create_gate_desc((uint64_t)de_handler);
    idt[1] = create_gate_desc((uint64_t)db_handler);
    idt[2] = create_gate_desc((uint64_t)nmi_handler);
    idt[3] = create_gate_desc((uint64_t)bp_handler);
    idt[4] = create_gate_desc((uint64_t)of_handler);
    idt[5] = create_gate_desc((uint64_t)br_handler);
    idt[6] = create_gate_desc((uint64_t)ud_handler);
    idt[7] = create_gate_desc((uint64_t)nm_handler);
    idt[8] = create_gate_desc((uint64_t)df_handler);
    idt[10] = create_gate_desc((uint64_t)ts_handler);
    idt[11] = create_gate_desc((uint64_t)np_handler);
    idt[12] = create_gate_desc((uint64_t)ss_handler);
    idt[13] = create_gate_desc((uint64_t)gp_handler);
    idt[14] = create_gate_desc((uint64_t)pf_handler);
    idt[16] = create_gate_desc((uint64_t)mf_handler);
    idt[17] = create_gate_desc((uint64_t)ac_handler);
    idt[18] = create_gate_desc((uint64_t)mc_handler);
    idt[19] = create_gate_desc((uint64_t)xm_handler);
    // register user-defined handlers
    //idt[32] = create_gate_desc((uint64_t)timer_handler);
    //idt[33] = create_gate_desc((uint64_t)ps2_handler);
    //idt[36] = create_gate_desc((uint64_t)com1_handler);

    uint16_t size_idt = sizeof(struct gate_desc) * 256 - 1;
    __asm__ volatile("sub rsp,0x10\n\t"
                     "mov [rsp],%0\n\t"
                     "mov [rsp+2],%1\n\t"
                     "lidt [rsp]\n\t"
                     "add rsp,0x10\n\t"
                    :
                    : "r" (size_idt), "r" (idt));

}
