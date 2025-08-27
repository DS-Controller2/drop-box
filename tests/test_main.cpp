#include <gtest/gtest.h>

// Simple test to verify Google Test setup
TEST(BasicTest, SanityCheck) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

// You don't need a main function here; GTest::gtest_main provides it.