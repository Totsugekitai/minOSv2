#include <stdint.h>
#include "util.h"

#define PORT    (uint8_t)0x3f8

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

extern char keycode;

void init_pic(void)
{
    io_out8(PIC0_ICW1, 0x11);
    io_out8(PIC0_ICW2,   32); // IRQ0-7をINT 0x20-0x27にマップ
    io_out8(PIC0_ICW3,    4);
    io_out8(PIC0_ICW4, 0x01);
    io_out8(PIC0_IMR,  0xff);

    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2,   40); // IRQ8-15をINT 0x28-0x2fにマップ
    io_out8(PIC1_ICW3,    2);
    io_out8(PIC1_ICW4, 0x01);
    io_out8(PIC1_IMR,  0xff);

    io_out8(PIC0_IMR,  0xee); // タイマとCOM1以外はMask
    io_out8(PIC1_IMR,  0xff);
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
    io_out8(PORT + 1, 0x0d); // 割り込み許可
    io_sti();
}

//uint32_t serial_received(void)
//{
//    return io_in8(PORT + 5) & 1;
//}
//
//void wait_serial_input(void)
//{
//    while (keycode == 0) {
//        __asm__ volatile("hlt");
//    }
//}
//
//uint8_t receive_serial_input(void)
//{
//    while (keycode == 0) {
//        __asm__ volatile("hlt");
//    }
//
//    uint8_t c = keycode;
//
//    return c;
//}
//
//uint8_t read_serial(void)
//{
//    while (serial_received() == 0);
//    return io_in8(PORT);
//}

static uint8_t serial_thr_empty(void)
{
    return io_in8(PORT + 5) & 0x20;
}

static void write_serial(uint8_t c)
{
    while (!serial_thr_empty());
    //while (serial_thr_empty() == 0);
    io_out8(PORT, c);
}

void puts_serial(const char *s)
{
    int i = 0;
    io_cli();
    while (s[i] != '\0') {
        write_serial(s[i]);
        i++;
    }
    io_sti();
}

