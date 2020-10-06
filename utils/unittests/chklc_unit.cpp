//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(chklc_unit, chklc) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(chklc_unit, chklc2) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(chklc_unit, cmp_glib_version) { // NOLINT
  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
