#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void halt(void);
void busy_loop(void);
void io_cli(void);
void io_sti(void);
uint8_t io_in8(uint16_t port);
void io_out8(uint16_t port, uint8_t val);
uint64_t mypow(uint64_t num, uint64_t p);
void init_bss(void);

#endif
