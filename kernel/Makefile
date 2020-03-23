TARGET		= kernel.elf
CFLAGS		= -g -O0 \
			  -masm=intel \
			  -Wall -Wextra -Wno-unused-parameter \
			  -I /usr/include \
			  -nostdinc -nostdlib \
			  -fno-builtin -fno-common -fno-stack-protector -fno-pic \
			  -mno-red-zone
LDFLAGS		= -s -x -T kernel.lds -Map kernel.map -e entry_point

OBJS		= kernel.o \
			  util.o \
			  graphics.o \
			  serial.o \
			  mm.o mm_asm.o \
			  int_handler.o \

.PHONY: clean

$(TARGET): $(OBJS)
	ld $(LDFLAGS) -o $@ $+

int_handler.o: int_handler.c
	gcc $(CFLAGS) -mgeneral-regs-only -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.S
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.asm
	nasm -f elf64 -o $@ $<

clean:
	rm -f *.o *.elf *.map
