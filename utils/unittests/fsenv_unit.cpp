//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(fsenv_unit, FSENVgetenv) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(fsenv_unit, FSENVprintenv) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(fsenv_unit, FSENVsetSUBJECTS_DIR) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(fsenv_unit, FSENVgetSUBJECTS_DIR) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(fsenv_unit, FSENVfree) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
