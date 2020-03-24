#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define PORT1       0x3f8
#define PORT2       0x2f8

#define PIC0_ICW1   0x20
#define PIC0_OCW2   0x20
#define PIC0_IMR    0x21
#define PIC0_ICW2   0x21
#define PIC0_ICW3   0x21
#define PIC0_ICW4   0x21
#define PIC1_ICW1   0xa0
#define PIC1_OCW2   0xa0
#define PIC1_IMR    0xa1
#define PIC1_ICW2   0xa1
#define PIC1_ICW3   0xa1
#define PIC1_ICW4   0xa1
#define PIC_EOI     0x20

void init_pic(void);
void init_serial(void);
void puts_serial(const char *s);
void putn_serial(uint64_t num);

#endif
