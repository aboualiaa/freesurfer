//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(filter_unit, ImageNitShiFilter) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(filter_unit, ImageGreyErode) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(filter_unit, ImageGreyDilate) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(filter_unit, ImageLOGFilter) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(filter_unit, ImageDOGFilter) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
