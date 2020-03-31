#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

struct gate_desc {
    uint32_t low;
    uint32_t mid;
    uint64_t high;
};

void init_idt(void);
extern void set_idt(struct gate_desc *idt_addr, uint16_t size);

#endif
