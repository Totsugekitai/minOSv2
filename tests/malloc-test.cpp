#include "gtest/gtest.h"

extern "C" {
    #include "../kernel/mm.h"
    #include <stdlib.h>
    #include <stdint.h>
}

#include <iostream>
class mallocTest : public ::testing::Test {
protected:
    virtual void setUp() {
        //struct malloc_header *heap_start = (struct malloc_header *)aligned_alloc(16, 0x100000);
        //init_kheap(heap_start);
    }
    virtual void TearDown() {}
};

TEST_F(mallocTest, malloc_alloc_test) {
    struct malloc_header *heap_start = (struct malloc_header *)aligned_alloc(16, HEAP_SIZE);
    init_kheap(heap_start);
    ASSERT_EQ(0, ((uint64_t)heap_start % 16));
    char *char_alloc = (char *)kmalloc(sizeof(char));
    int *int_alloc = (int *)kmalloc(sizeof(int));
    long *long_alloc = (long *)kmalloc(sizeof(long));
    int *int_array_alloc = (int *)kmalloc(sizeof(int) * 4);
    char *char_array_alloc = (char *)kmalloc(sizeof(char) * 17);

    ASSERT_EQ(0, ((uint64_t)char_alloc % 16));
    ASSERT_EQ(0, ((uint64_t)int_alloc % 16));
    ASSERT_EQ(0, ((uint64_t)long_alloc % 16));
    ASSERT_EQ(0, ((uint64_t)int_array_alloc % 16));
    ASSERT_EQ(0, ((uint64_t)char_array_alloc % 16));
}
