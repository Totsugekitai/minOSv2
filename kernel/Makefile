TARGET		= kernel.elf
CFLAGS		= -g -O0 \
			  -Wall -Wextra -Wno-unused-parameter \
			  -I /usr/include \
			  -nostdinc -nostdlib \
			  -fno-builtin -fno-common -fno-stack-protector \
			  -mno-red-zone
LDFLAGS		= -T kernel.lds -Map kernel.map -e entry_point

OBJS		= kernel.o \
			  graphics.o

.PHONY: clean

$(TARGET): $(OBJS)
	ld $(LDFLAGS) -o $@ $+

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.elf *.map
