#include <stdint.h>
#include "../util.h"
#include "serial.h"

void init_pic(void)
{
    io_cli();
    io_out8(PIC0_ICW1, 0x11);
    io_out8(PIC0_ICW2,   32);   // IRQ0-7をINT 0x20-0x27にマップ
    io_out8(PIC0_ICW3,    4);
    io_out8(PIC0_ICW4, 0x01);
    io_out8(PIC0_IMR,  0xff);

    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2,   40);   // IRQ8-15をINT 0x28-0x2fにマップ
    io_out8(PIC1_ICW3,    2);
    io_out8(PIC1_ICW4, 0x01);
    io_out8(PIC1_IMR,  0xff);

    io_out8(PIC0_IMR,  0xe6);   // タイマとCOM1〜4以外はMask
    io_out8(PIC1_IMR,  0xff);
    io_sti();
}

extern int gpd;
uint16_t comport;
void init_serial(void)
{
    if (gpd) {
        comport = PORT2;
    } else {
        comport = PORT1;
    }

    io_cli();
    io_out8(comport + 1, 0x00);
    io_out8(comport + 3, 0x80);
    io_out8(comport + 0, 0x01);    // 115200 baud rete
    io_out8(comport + 1, 0x00);
    io_out8(comport + 3, 0x03);
    io_out8(comport + 2, 0xC7);
    io_out8(comport + 4, 0x0B);
    io_out8(comport + 1, 0x0d);    // 割り込み許可
    io_sti();
}

static uint8_t serial_thr_empty(void)
{
    return io_in8(comport + 5) & 0x20;
}

static void write_serial(uint8_t c)
{
    while (!serial_thr_empty());
    io_out8(comport, c);
}

void puts_serial(const char *s)
{
    io_cli();
    for (int i = 0; s[i] != '\0'; i++) {
        write_serial(s[i]);
    }
    io_sti();
}

void putn_serial(uint64_t num)
{
    uint64_t i;
    uint64_t numarr[16];
    char numchararr[19];
    for (i = 0; i < 16; i++) {
        numarr[i] = (num & (mypow(2, 0 + i * 4) + mypow(2, 1 + i * 4) + mypow(2, 2 + i * 4) + mypow(2, 3 + i * 4))) >> i * 4;
    }
    numchararr[0] = '0';
    numchararr[1] = 'x';
    numchararr[18] = '\0';
    for (i = 0; i < 16; i++) {
        if (numarr[i] == 0x0) {
            numchararr[17 - i] = '0';
        }
        if (numarr[i] == 0x1) {
            numchararr[17 - i] = '1';
        }
        if (numarr[i] == 0x2) {
            numchararr[17 - i] = '2';
        }
        if (numarr[i] == 0x3) {
            numchararr[17 - i] = '3';
        }
        if (numarr[i] == 0x4) {
            numchararr[17 - i] = '4';
        }
        if (numarr[i] == 0x5) {
            numchararr[17 - i] = '5';
        }
        if (numarr[i] == 0x6) {
            numchararr[17 - i] = '6';
        }
        if (numarr[i] == 0x7) {
            numchararr[17 - i] = '7';
        }
        if (numarr[i] == 0x8) {
            numchararr[17 - i] = '8';
        }
        if (numarr[i] == 0x9) {
            numchararr[17 - i] = '9';
        }
        if (numarr[i] == 0xa) {
            numchararr[17 - i] = 'a';
        }
        if (numarr[i] == 0xb) {
            numchararr[17 - i] = 'b';
        }
        if (numarr[i] == 0xc) {
            numchararr[17 - i] = 'c';
        }
        if (numarr[i] == 0xd) {
            numchararr[17 - i] = 'd';
        }
        if (numarr[i] == 0xe) {
            numchararr[17 - i] = 'e';
        }
        if (numarr[i] == 0xf) {
            numchararr[17 - i] = 'f';
        }
    }
    puts_serial(numchararr);
}
