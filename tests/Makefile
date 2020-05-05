.RECIPEPREFIX = >

CFLAGS = -c -O0 \
	-masm=intel \
	-Wall -Wextra -Wno-unused-parameter \
	-I /usr/include \
	-nostdinc -nostdlib \
	-fno-builtin -fno-common -fno-stack-protector \
	-mno-red-zone

CPPFLAGS = -I. -pthread

.PHONY: default clean

default:
> make malloc-test

malloc-test: ../kernel/mm.c malloc-test.cpp
> clang -DMINOS_TEST $(CFLAGS) ../kernel/mm.c
> clang++ malloc-test.cpp ../../googletest-release-1.10.0/googletest/src/gtest_main.cc gtest/gtest-all.cc $(CPPFLAGS) mm.o -o $@
> ./$@

clean:
> @rm -rf *.o
> @rm -rf malloc-test
