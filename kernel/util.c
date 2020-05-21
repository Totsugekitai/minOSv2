#include <stdint.h>
#include <stddef.h>
#include "util.h"

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
    __asm__ volatile("nop");
    __asm__ volatile("hlt");
}

void clihlt(void)
{
    __asm__ volatile("cli");
    __asm__ volatile("nop");
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

void *memset(void *buf, int ch, unsigned long n)
{
    char *b = (char *)buf;
    for (unsigned long i = 0; i < n; i++) {
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
    uint64_t size = (uint64_t)&__bss_end - (uint64_t)&__bss_start;
    for (uint64_t i = 0; i < size; i++) {
        (&__bss_start)[i] = 0;
    }
}

void flush_queue_char(struct queue_char *que)
{
    for (int i = 0; i < QUEUE_SIZE; i++) {
        que->data[i] = 0;
    }
    que->head = 0;
    que->num = 0;
}

int queue_char_isempty(struct queue_char *que)
{
    return !que->num;
}

int queue_char_isfull(struct queue_char *que)
{
    return que->num == QUEUE_SIZE;
}

char queue_char_head(struct queue_char *que)
{
    return que->data[que->head];
}

int enqueue_char(struct queue_char *que, char c)
{
    if (que->num < QUEUE_SIZE) {
        que->data[(que->head + que->num) % QUEUE_SIZE] = c;
        que->num++;
        return 1;
    } else {
        return 0;
    }
}

int enqueue_str(struct queue_char *que, char *s)
{
    int success, i;
    for (i = 0; s[i] != 0; i++) {
        success = enqueue_char(que, s[i]);
        if (!success) {
            return 0;
        }
    }
    success = enqueue_char(que, s[i]);
    if (!success) {
        return 0;
    }
    return 1;
}

int dequeue_char(struct queue_char *que, char *c)
{
    if (que->num > 0) {
        *c = que->data[que->head];
        que->head = (que->head + 1) % QUEUE_SIZE;
        que->num--;
        return 1;
    } else {
        return 0;
    }
}

int dequeue_str(struct queue_char *que, char *buf)
{
    char dust;
    for (int i = 0; queue_char_head(que) == 0; i++) {
        dequeue_char(que, &dust);
    }
    for (int i = 0; dequeue_char(que, &buf[i]); i++) {
        if (buf[i]) {
            continue;
        } else {
            return 1;
        }
    }
    return 0;
}

int strcmp(const char *s1, const char *s2)
{
    int s1len = 0, s2len = 0;
    for (int i = 0; s1[i] != 0; i++) {
        s1len++;
    }
    for (int i = 0; s2[i] != 0; i++) {
        s2len++;
    }
    if (s1len == s2len) {
        return strncmp(s1, s2, s1len);
    } else {
        if (s1len > s2len) {
            return 1;
        } else {
            return -1;
        }
    }
}

int strncmp(const char *s1, const char *s2, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        if (s1[i] > s2[i]) {
            return 1;
        } else if (s1[i] < s2[i]) {
            return -1;
        } else {
            continue;
        }
    }
    return 0;
}

char *strcpy(char *s1, const char *s2)
{
    int i;
    for (i = 0; s2[i] != 0; i++) {
        s1[i] = s2[i];
    }
    s1[i] = s2[i];
    return s1;
}

uint64_t strlen(const char *s)
{
    int count = 0;
    while (s[count] != 0) {
        count++;
    }
    return count;
}
