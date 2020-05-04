#include "gtest/gtest.h"

extern "C" {
#include "../kernel/mm.h"
}

class mallocTest : public ::testing::Test {
protected:
    virtual void setUp() {}
    virtual void TearDown() {}
};

TEST_F(mallocTest, malloc_test) {}
