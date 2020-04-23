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

void stihlt(void)
{
    __asm__ volatile("sti");
    __asm__ volatile("hlt");
}

uint64_t mypow(uint64_t num, uint64_t p)
{
    uint64_t ans = 1;
    for (uint64_t i = 0; i < p; i++) {
        ans *= num;
    }
    return ans;
}

void *mymemset(void *buf, int ch, int n)
{
    char *b = (char *)buf;
    for (int i = 0; i < n; i++) {
        b[i] = ch;
    }
    return (void *)b;
}

void *memcpy(void *buf1, const void *buf2, unsigned long n)
{
    char *c1 = (char *)buf1;
    char *c2 = (char *)buf2;
    for (unsigned long i = 0; i < n; i++) {
        c1[i] = c2[i];
    }
    return buf1;
}

extern uint64_t __bss_start, __bss_end;
void init_bss(void)
{
    int size = (int)(&__bss_end - &__bss_start);
    for (int i = 0; i < size; i++) {
        (&__bss_start)[i] = 0;
    }
}
