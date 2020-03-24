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

    io_out8(PIC0_IMR,  0xee);   // タイマとCOM1以外はMask
    io_out8(PIC1_IMR,  0xff);
    io_sti();
}

void init_serial(void)
{
    io_cli();
    io_out8(PORT + 1, 0x00);
    io_out8(PORT + 3, 0x80);
    io_out8(PORT + 0, 0x01);
    io_out8(PORT + 1, 0x00);
    io_out8(PORT + 3, 0x03);
    io_out8(PORT + 2, 0xC7);
    io_out8(PORT + 4, 0x0B);
    io_out8(PORT + 1, 0x0d);    // 割り込み許可
    io_sti();
}

static uint8_t serial_thr_empty(void)
{
    return io_in8(PORT + 5) & 0x20;
}

static void write_serial(uint8_t c)
{
    while (!serial_thr_empty()) { halt(); }
    io_out8(PORT, c);
}

void puts_serial(const char *s)
{
    io_cli();
    for (int i = 0; s[i] != '\0'; i++) {
        write_serial(s[i]);
    }
    io_sti();
}

