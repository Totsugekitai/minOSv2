#include <stdint.h>

// ココらへんの単純な関数はアトでマクロにするかも
void halt(void)
{
    while (1) {
        __asm__ volatile("hlt");
    }
}

void busy_loop(void)
{
    while (1);
}

void io_cli(void)
{
    __asm__ volatile("cli");
}

void io_sti(void)
{
    __asm__ volatile("sti");
}

uint8_t io_in8(uint16_t port)
{
    uint8_t val = 0;
    __asm__ volatile("movw dx,%1\n\t"
                     "in   al,dx\n\t"
                     "movb %0,al\n\t"
                    : "=m" (val)
                    : "r" (port)
                    : "rax", "rdx");
    return val;
}

void io_out8(uint16_t port, uint8_t val)
{
    __asm__ volatile("movw dx,%0\n\t"
                     "movb al,%1\n\t"
                     "out dx,al\n\t"
                    :
                    : "r" (port), "r" (val)
                    : "rax", "rdx");
}

uint64_t mypow(uint64_t num, uint64_t p)
{
    uint64_t ans = 1;
    for (uint64_t i = 0; i < p; i++) {
        ans *= num;
    }
    return ans;
}


extern uint64_t __bss_start, __bss_end;
void init_bss(void)
{
    int size = (int)(&__bss_end - &__bss_start);
    for (int i = 0; i < size; i++) {
        (&__bss_start)[i] = 0;
    }
}
