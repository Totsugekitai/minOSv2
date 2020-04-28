#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define UNUSED(x) ((void)x)

#define QUEUE_SIZE (1600)
struct queue_char {
    int head;
    int num;
    char data[QUEUE_SIZE];  // 80 char 20 line
};

void halt(void);
void busy_loop(void);
void io_cli(void);
void io_sti(void);
void stihlt(void);
extern uint8_t io_in8(uint16_t port);
extern void io_out8(uint16_t port, uint8_t val);
extern uint32_t io_in32(uint16_t port);
extern void io_out32(uint16_t port, uint32_t val);
uint64_t mypow(uint64_t num, uint64_t p);
void *memset(void *buf, int ch, unsigned long n);
void *memcpy(void *buf1, const void *buf2, unsigned long n);
int is_aligned(void *addr, int align);
void *align(void *addr, int align);
void init_bss(void);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned long n);
char *strcpy(char *s1, const char *s2);

int queue_char_isempty(struct queue_char *que);
int queue_char_isfull(struct queue_char *que);
char queue_char_head(struct queue_char *que);
int enqueue_char(struct queue_char *que, char c);
int enqueue_str(struct queue_char *que, char *s);
int dequeue_char(struct queue_char *que, char *c);
int dequeue_str(struct queue_char *que, char *buf);

#endif
