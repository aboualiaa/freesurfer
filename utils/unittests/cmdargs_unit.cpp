//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(cmdargs_unit, CMDargNErr) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDsingleDash) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDisFlag) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDnthIsArg) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDstringMatch) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDprintUsage) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDusageExit) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(cmdargs_unit, CMDprintHelp) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
