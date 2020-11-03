//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(log_unit, throwExceptions) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(log_unit, setErrorLog) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(log_unit, writeToErrorLog) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(log_unit, errorExit) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(log_unit, ErrorExit) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(log_unit, ErrorPrintf) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
