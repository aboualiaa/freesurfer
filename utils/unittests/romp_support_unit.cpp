//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(romp_support_unit, ROMP_main_started) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_show_stats) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_countGoParallel) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_pf_begin) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_if_parallel1) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_if_parallel2) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_pf_end) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_pflb_begin) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_pflb_end) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_Distributor_begin) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(romp_support_unit, ROMP_Distributor_end) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
