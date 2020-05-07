#include "gtest/gtest.h"

extern "C" {
    #include "../kernel/mm.h"
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdio.h>
}

#include <iostream>
class mallocTest : public ::testing::Test {
protected:
    virtual void setUp() {}
    virtual void TearDown() {}
};

TEST_F(mallocTest, kmalloc_kfree_test) {
    struct malloc_header *heap_start = (struct malloc_header *)aligned_alloc(HEAP_SIZE, HEAP_SIZE);
    printf("allocated area: %p\n", heap_start);
    init_kheap(heap_start);
    ASSERT_EQ(0, ((uint64_t)heap_start % 16));

    char *char_alloc = (char *)kmalloc(sizeof(char));
    *char_alloc = 'A';
    int *int_alloc = (int *)kmalloc(sizeof(int));
    *int_alloc = 0xeeeeeeee;
    long *long_alloc = (long *)kmalloc(sizeof(long));
    *long_alloc = 0xddddddddddddddddull;
    int *int_array_alloc = (int *)kmalloc(sizeof(int) * 4);
    for (int i = 0; i < 4; i++) {
        int_array_alloc[i] = 0xcccccccc;
    }
    char *char_array_alloc = (char *)kmalloc(sizeof(char) * 17);
    for (int i = 0; i < 17; i++) {
        char_array_alloc[i] = 'B';
    }

    ASSERT_EQ(1, is_aligned(char_alloc, 16));
    ASSERT_EQ(1, is_aligned(int_alloc, 16));
    ASSERT_EQ(1, is_aligned(long_alloc, 16));
    ASSERT_EQ(1, is_aligned(int_array_alloc, 16));
    ASSERT_EQ(1, is_aligned(char_array_alloc, 16));

    ASSERT_EQ('A', *char_alloc);
    ASSERT_EQ(0xeeeeeeee, *int_alloc);
    ASSERT_EQ(0xddddddddddddddddull, *long_alloc);
    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(0xcccccccc, int_array_alloc[i]);
    }
    for (int i = 0; i < 17; i++) {
        ASSERT_EQ('B', char_array_alloc[i]);
    }

    ASSERT_EQ(1, count_free_block());
    kfree(int_alloc);
    ASSERT_EQ(2, count_free_block());
    kfree(int_array_alloc);
    ASSERT_EQ(3, count_free_block());
    kfree(char_alloc);
    ASSERT_EQ(3, count_free_block());
    kfree(long_alloc);
    ASSERT_EQ(2, count_free_block());
    kfree(char_array_alloc);
    ASSERT_EQ(1, count_free_block());

    uint64_t *aligned_4KB = (uint64_t *)kmalloc_alignas(sizeof(uint64_t) * 32, 0x1000);
    for (int i = 0; i < 32; i++) {
        aligned_4KB[i] = 0xdeadbeef;
    }
    for (int i = 0; i < 32; i++) {
        ASSERT_EQ(0xdeadbeef, aligned_4KB[i]);
    }
    ASSERT_EQ(17, ((struct malloc_header *)aligned_4KB)[-1].size);
    ASSERT_EQ(2, count_free_block());
    printf("aligned address: %p\n", aligned_4KB);
    ASSERT_EQ(1, is_aligned(aligned_4KB, 0x1000));
    kfree(aligned_4KB);
    ASSERT_EQ(1, count_free_block());
}
