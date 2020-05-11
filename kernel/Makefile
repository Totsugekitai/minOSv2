.RECIPEPREFIX = >
TARGET = kernel.elf
CFLAGS = -g -O0 \
	  -masm=intel \
	  -Wall -Wextra -Wno-unused-parameter \
	  -I /usr/include \
	  -I /usr/lib/clang/*/include \
	  -nostdinc -nostdlib \
	  -fno-builtin -fno-common -fno-stack-protector -fno-pic \
	  -mno-red-zone
LDFLAGS = -s -x -T kernel.lds -Map kernel.map -e entry_point

OBJS = kernel.o \
	  util.o util_asm.o \
	  graphics.o \
	  mm.o mm_asm.o \
	  task.o task_asm.o \
	  interrupt.o interrupt_asm.o int_handler.o \
	  device/serial.o \
	  device/pci.o \
	  device/ahci.o \
	  fs/ext2.o \
	  app/app.o #app/console.o \

.PHONY: clean

$(TARGET): $(OBJS)
> ld.lld $(LDFLAGS) -o $@ $+

int_handler.o: int_handler.c
> clang $(CFLAGS) -mno-implicit-float -c -o $@ $<

%.o: %.c
> clang $(CFLAGS) -c -o $@ $<

%.o: %.S
> clang $(CFLAGS) -c -o $@ $<

clean:
> @rm -f *.o *.gch *.elf *.map
> @rm -f device/*.o
> @rm -f fs/*.o
> @rm -f app/*.o
